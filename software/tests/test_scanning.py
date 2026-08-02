from collections.abc import Iterator
from datetime import UTC, datetime

import pytest

from radiance3d.interfaces import (
    MeasurementReading,
    MeasurementValidity,
    PositionConfidence,
    PositionKind,
    PositionReport,
)
from radiance3d.scanning import (
    AxisScan,
    RasterScanConfig,
    ScanCoordinator,
    ScanPoint,
    raster_points,
)

NOW = datetime(2026, 7, 30, tzinfo=UTC)


class FakeMotion:
    protocol_version = "1"

    def __init__(self, *, trusted: bool = True) -> None:
        self._trusted = trusted
        self.moves: list[tuple[float, float, float | None]] = []

    def home(self) -> PositionReport:
        return self._report(0.0, 0.0)

    def move_to(
        self,
        azimuth_deg: float,
        elevation_deg: float,
        speed_deg_per_s: float | None = None,
    ) -> PositionReport:
        self.moves.append((azimuth_deg, elevation_deg, speed_deg_per_s))
        return self._report(azimuth_deg, elevation_deg)

    def position(self) -> PositionReport:
        return self._report(0.0, 0.0)

    def emergency_stop(self) -> None:
        self._trusted = False

    def _report(self, azimuth_deg: float, elevation_deg: float) -> PositionReport:
        confidence = PositionConfidence.TRUSTED if self._trusted else PositionConfidence.UNTRUSTED
        return PositionReport(
            azimuth_deg,
            elevation_deg,
            NOW,
            PositionKind.COMMANDED,
            confidence,
            motion_complete=self._trusted,
        )


class FakeMeasurement:
    source_id = "fake-receiver"

    def __init__(self, readings: Iterator[MeasurementReading]) -> None:
        self._readings = readings
        self.timeouts: list[float] = []

    def measure(self, timeout_s: float) -> MeasurementReading:
        self.timeouts.append(timeout_s)
        return next(self._readings)


def reading(
    value: float,
    validity: MeasurementValidity = MeasurementValidity.VALID,
) -> MeasurementReading:
    return MeasurementReading(value, "ADC_counts", NOW, "fake-receiver", validity)


def config(**changes: object) -> RasterScanConfig:
    defaults: dict[str, object] = {
        "azimuth": AxisScan(0.0, 2.0, 1.0, 0.0, 359.0),
        "elevation": AxisScan(-1.0, 1.0, 1.0, -90.0, 90.0),
        "settle_time_s": 0.5,
    }
    defaults.update(changes)
    return RasterScanConfig(**defaults)  # type: ignore[arg-type]


def test_raster_reverses_alternate_rows_and_uses_inclusive_ranges() -> None:
    points = list(raster_points(config()))

    assert [(point.azimuth_deg, point.elevation_deg) for point in points] == [
        (0.0, -1.0),
        (1.0, -1.0),
        (2.0, -1.0),
        (2.0, 0.0),
        (1.0, 0.0),
        (0.0, 0.0),
        (0.0, 1.0),
        (1.0, 1.0),
        (2.0, 1.0),
    ]
    assert [point.sequence_number for point in points] == list(range(9))


def test_axis_scan_rejects_requested_travel_outside_machine_limits() -> None:
    with pytest.raises(ValueError, match="exceeds configured axis limits"):
        AxisScan(-1.0, 359.0, 1.0, 0.0, 359.0)


def test_scan_configuration_rejects_unknown_averaging_and_zero_timeout() -> None:
    with pytest.raises(ValueError, match="averaging_method"):
        config(averaging_method="mode")
    with pytest.raises(ValueError, match="measurement_timeout_s"):
        config(measurement_timeout_s=0.0)


def test_capture_preserves_raw_and_rejected_readings_before_averaging() -> None:
    motion = FakeMotion()
    receiver = FakeMeasurement(
        iter(
            [
                reading(999.0, MeasurementValidity.INVALID),
                reading(10.0),
                reading(14.0),
            ]
        )
    )
    sleeps: list[float] = []
    scan_config = config(
        samples_per_position=2,
        averaging_method="mean",
        retry_count=1,
        measurement_timeout_s=1.25,
        speed_deg_per_s=3.0,
    )

    capture = ScanCoordinator(motion, receiver, sleep=sleeps.append).capture_point(
        ScanPoint(4, 2.0, 0.0),
        scan_config,
    )

    assert capture.aggregate_value == 12.0
    assert [item.value for item in capture.raw_readings] == [10.0, 14.0]
    assert [item.value for item in capture.rejected_readings] == [999.0]
    assert sleeps == [0.5]
    assert receiver.timeouts == [1.25, 1.25, 1.25]
    assert motion.moves == [(2.0, 0.0, 3.0)]


def test_capture_refuses_untrusted_position() -> None:
    coordinator = ScanCoordinator(FakeMotion(trusted=False), FakeMeasurement(iter([reading(1.0)])))

    with pytest.raises(RuntimeError, match="trusted measurement boundary"):
        coordinator.capture_point(ScanPoint(0, 0.0, 0.0), config())


def test_position_report_does_not_allow_untrusted_ready_state() -> None:
    with pytest.raises(ValueError, match="cannot be a measurement boundary"):
        PositionReport(
            0.0,
            0.0,
            NOW,
            PositionKind.COMMANDED,
            PositionConfidence.UNTRUSTED,
            motion_complete=True,
        )
