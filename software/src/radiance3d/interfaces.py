"""Stable host-side boundaries for motion and RF measurement devices."""

from __future__ import annotations

from dataclasses import dataclass
from datetime import datetime
from enum import Enum
from math import isfinite
from typing import Protocol, runtime_checkable


class PositionConfidence(str, Enum):  # noqa: UP042
    """Whether the controller's open-loop position is safe to use."""

    TRUSTED = "trusted"
    UNTRUSTED = "untrusted"


class PositionKind(str, Enum):  # noqa: UP042
    """Origin of a reported position."""

    COMMANDED = "commanded"
    OBSERVED = "observed"


class MeasurementValidity(str, Enum):  # noqa: UP042
    """Receiver-adapter assessment of one native reading."""

    VALID = "valid"
    INVALID = "invalid"
    TIMEOUT = "timeout"


@dataclass(frozen=True)
class PositionReport:
    """Controller position with explicit origin and confidence."""

    azimuth_deg: float
    elevation_deg: float
    timestamp: datetime
    kind: PositionKind
    confidence: PositionConfidence
    motion_complete: bool
    warnings: tuple[str, ...] = ()

    def __post_init__(self) -> None:
        if not isfinite(self.azimuth_deg) or not isfinite(self.elevation_deg):
            raise ValueError("reported angles must be finite")
        if self.timestamp.tzinfo is None:
            raise ValueError("position timestamp must include a timezone")
        if self.confidence is PositionConfidence.UNTRUSTED and self.motion_complete:
            raise ValueError("an untrusted position cannot be a measurement boundary")


@dataclass(frozen=True)
class MeasurementReading:
    """One unmodified reading returned by a measurement adapter."""

    value: float
    unit: str
    timestamp: datetime
    source_id: str
    validity: MeasurementValidity
    warnings: tuple[str, ...] = ()

    def __post_init__(self) -> None:
        if not isfinite(self.value):
            raise ValueError("measurement value must be finite")
        if not self.unit.strip():
            raise ValueError("measurement unit must not be empty")
        if not self.source_id.strip():
            raise ValueError("measurement source identifier must not be empty")
        if self.timestamp.tzinfo is None:
            raise ValueError("measurement timestamp must include a timezone")
        if len(self.warnings) != len(set(self.warnings)):
            raise ValueError("measurement warnings must not contain duplicates")


@runtime_checkable
class MotionController(Protocol):
    """Public host API for interchangeable motion-controller implementations."""

    @property
    def protocol_version(self) -> str:
        """Return the negotiated motion protocol version."""

    def home(self) -> PositionReport:
        """Home both axes and return the resulting commanded-position report."""

    def move_to(
        self,
        azimuth_deg: float,
        elevation_deg: float,
        speed_deg_per_s: float | None = None,
    ) -> PositionReport:
        """Move within configured limits and wait for motion completion."""

    def position(self) -> PositionReport:
        """Return the controller's current open-loop position report."""

    def emergency_stop(self) -> None:
        """Stop motion and invalidate position confidence."""


@runtime_checkable
class MeasurementAdapter(Protocol):
    """Receiver-neutral interface; implementations preserve native units."""

    @property
    def source_id(self) -> str:
        """Return a stable identifier recorded with every reading."""

    def measure(self, timeout_s: float) -> MeasurementReading:
        """Return one reading or an explicit invalid/timeout reading."""
