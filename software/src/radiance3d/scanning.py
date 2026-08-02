"""Bounds-safe raster planning and receiver-neutral scan coordination."""

from __future__ import annotations

import statistics
import time
from collections.abc import Callable, Iterator
from dataclasses import dataclass
from math import isfinite
from typing import Literal

from radiance3d.interfaces import (
    MeasurementAdapter,
    MeasurementReading,
    MeasurementValidity,
    MotionController,
    PositionConfidence,
    PositionReport,
)

AveragingMethod = Literal["none", "mean", "median"]


def _require_finite(value: float, field: str) -> None:
    if not isfinite(value):
        raise ValueError(f"{field} must be finite")


def _axis_values(start: float, stop: float, step: float) -> tuple[float, ...]:
    """Build an inclusive sequence without accumulating binary floating-point error."""

    if step <= 0:
        raise ValueError("step size must be greater than zero")
    if start > stop:
        raise ValueError("axis start must not exceed axis stop")
    count = int((stop - start) // step)
    values = [start + index * step for index in range(count + 1)]
    tolerance = max(1e-9, step * 1e-9)
    if stop - values[-1] > tolerance:
        values.append(stop)
    else:
        values[-1] = stop
    return tuple(round(value, 12) for value in values)


@dataclass(frozen=True)
class AxisScan:
    """Inclusive scan range for one axis, in degrees."""

    start_deg: float
    stop_deg: float
    step_deg: float
    minimum_deg: float
    maximum_deg: float

    def __post_init__(self) -> None:
        for field in (
            "start_deg",
            "stop_deg",
            "step_deg",
            "minimum_deg",
            "maximum_deg",
        ):
            _require_finite(getattr(self, field), field)
        if self.minimum_deg > self.maximum_deg:
            raise ValueError("axis minimum must not exceed maximum")
        if self.start_deg < self.minimum_deg or self.stop_deg > self.maximum_deg:
            raise ValueError("scan range exceeds configured axis limits")
        _axis_values(self.start_deg, self.stop_deg, self.step_deg)

    @property
    def values(self) -> tuple[float, ...]:
        return _axis_values(self.start_deg, self.stop_deg, self.step_deg)


@dataclass(frozen=True)
class RasterScanConfig:
    """Version 1 raster and synchronization behavior."""

    azimuth: AxisScan
    elevation: AxisScan
    settle_time_s: float = 0.25
    samples_per_position: int = 1
    averaging_method: AveragingMethod = "none"
    measurement_timeout_s: float = 2.0
    retry_count: int = 0
    reverse_alternate_rows: bool = True
    speed_deg_per_s: float | None = None

    def __post_init__(self) -> None:
        for value, field in (
            (self.settle_time_s, "settle_time_s"),
            (self.measurement_timeout_s, "measurement_timeout_s"),
        ):
            _require_finite(value, field)
            if value < 0:
                raise ValueError(f"{field} must not be negative")
        if self.samples_per_position < 1:
            raise ValueError("samples_per_position must be at least one")
        if self.retry_count < 0:
            raise ValueError("retry_count must not be negative")
        if self.averaging_method not in {"none", "mean", "median"}:
            raise ValueError("averaging_method must be none, mean, or median")
        if self.averaging_method == "none" and self.samples_per_position != 1:
            raise ValueError("multiple samples require mean or median averaging")
        if self.measurement_timeout_s == 0:
            raise ValueError("measurement_timeout_s must be greater than zero")
        if self.speed_deg_per_s is not None:
            _require_finite(self.speed_deg_per_s, "speed_deg_per_s")
            if self.speed_deg_per_s <= 0:
                raise ValueError("speed_deg_per_s must be greater than zero")


@dataclass(frozen=True)
class ScanPoint:
    sequence_number: int
    azimuth_deg: float
    elevation_deg: float


@dataclass(frozen=True)
class PositionCapture:
    """Raw readings plus an optional aggregate that never replaces them."""

    point: ScanPoint
    position: PositionReport
    raw_readings: tuple[MeasurementReading, ...]
    rejected_readings: tuple[MeasurementReading, ...]
    aggregate_value: float | None
    aggregate_unit: str | None
    warnings: tuple[str, ...] = ()


def raster_points(config: RasterScanConfig) -> Iterator[ScanPoint]:
    """Yield elevation-major raster points, reversing alternate azimuth rows."""

    azimuth_values = config.azimuth.values
    sequence_number = 0
    for row, elevation_deg in enumerate(config.elevation.values):
        row_values = (
            tuple(reversed(azimuth_values))
            if config.reverse_alternate_rows and row % 2
            else azimuth_values
        )
        for azimuth_deg in row_values:
            yield ScanPoint(sequence_number, azimuth_deg, elevation_deg)
            sequence_number += 1


class ScanCoordinator:
    """Execute Version 1 move-settle-measure synchronization."""

    def __init__(
        self,
        motion: MotionController,
        measurement: MeasurementAdapter,
        *,
        sleep: Callable[[float], None] = time.sleep,
    ) -> None:
        self._motion = motion
        self._measurement = measurement
        self._sleep = sleep

    def capture_point(
        self,
        point: ScanPoint,
        config: RasterScanConfig,
    ) -> PositionCapture:
        position = self._motion.move_to(
            point.azimuth_deg,
            point.elevation_deg,
            config.speed_deg_per_s,
        )
        if position.confidence is not PositionConfidence.TRUSTED or not position.motion_complete:
            raise RuntimeError("motion controller did not establish a trusted measurement boundary")

        self._sleep(config.settle_time_s)
        accepted: list[MeasurementReading] = []
        rejected: list[MeasurementReading] = []
        attempts_remaining = config.samples_per_position + config.retry_count
        while len(accepted) < config.samples_per_position and attempts_remaining:
            reading = self._measurement.measure(config.measurement_timeout_s)
            attempts_remaining -= 1
            if reading.source_id != self._measurement.source_id:
                raise RuntimeError("measurement adapter returned an inconsistent source identifier")
            if reading.validity is MeasurementValidity.VALID:
                accepted.append(reading)
            else:
                rejected.append(reading)

        warnings = tuple(
            dict.fromkeys(
                warning for reading in (*accepted, *rejected) for warning in reading.warnings
            )
        )
        if len(accepted) < config.samples_per_position:
            return PositionCapture(
                point,
                position,
                tuple(accepted),
                tuple(rejected),
                None,
                None,
                (*warnings, "insufficient valid measurements"),
            )

        units = {reading.unit for reading in accepted}
        if len(units) != 1:
            raise RuntimeError("measurement adapter changed units within one scan position")
        aggregate = _aggregate(
            tuple(reading.value for reading in accepted), config.averaging_method
        )
        return PositionCapture(
            point,
            position,
            tuple(accepted),
            tuple(rejected),
            aggregate,
            accepted[0].unit,
            warnings,
        )


def _aggregate(values: tuple[float, ...], method: AveragingMethod) -> float:
    if method == "none":
        return values[0]
    if method == "mean":
        return statistics.fmean(values)
    return statistics.median(values)
