"""Public motion-controller client over an interchangeable protocol transport."""

from __future__ import annotations

from dataclasses import dataclass
from datetime import UTC, datetime
from math import isfinite
from time import monotonic

from radiance3d.interfaces import PositionConfidence, PositionKind, PositionReport
from radiance3d.transport import (
    ProtocolTransport,
    ResponseCorrelationError,
    TransportError,
    TransportTimeout,
    parse_fields,
)


class ControllerCommandError(TransportError):
    """Structured fault returned by the motion controller."""

    def __init__(self, response: str) -> None:
        super().__init__(response)
        self.response = response
        parts = response.split()
        self.code = next((part for part in parts[1:] if not part.startswith("ID=")), "UNKNOWN")


@dataclass(frozen=True)
class AxisConfiguration:
    axis: str
    full_steps_per_revolution: int
    microsteps: int
    gear_ratio: float
    minimum_angle_deg: float
    maximum_angle_deg: float
    maximum_speed_deg_per_s: float
    acceleration_deg_per_s2: float
    rms_current_ma: int
    hold_current_percent: int
    home_offset_deg: float


@dataclass(frozen=True)
class AxisState:
    axis: str
    commanded_position_deg: float
    internal_steps: int
    target_steps: int
    trusted: bool
    trust_loss_reason: str
    homed: bool
    moving: bool
    enabled: bool
    home_switch_active: bool
    homing_phase: str
    fault: str
    last_completed_command: int


@dataclass(frozen=True)
class DriverCapabilities:
    azimuth_present: bool
    elevation_present: bool
    uart_diagnostics: bool


@dataclass(frozen=True)
class DriverDiagnostics:
    axis: str
    connected: bool
    enabled: bool
    fault: str
    overtemperature_warning: bool
    overtemperature_shutdown: bool
    undervoltage: bool
    reset_detected: bool
    open_load_a: bool
    open_load_b: bool
    current_scale: int


class PhysicalMotionController:
    """Synchronous host API for either a physical or simulated protocol transport."""

    def __init__(
        self,
        transport: ProtocolTransport,
        *,
        motion_timeout_s: float = 120.0,
        event_poll_s: float = 0.25,
    ) -> None:
        if motion_timeout_s <= 0.0 or event_poll_s <= 0.0:
            raise ValueError("motion timeout and event poll interval must be positive")
        self._transport = transport
        self._motion_timeout_s = motion_timeout_s
        self._event_poll_s = event_poll_s
        self._protocol_version = "1"

    @property
    def protocol_version(self) -> str:
        return self._protocol_version

    def connect(self) -> None:
        self._transport.connect()

    def disconnect(self) -> None:
        self._transport.disconnect()

    def reconnect(self) -> None:
        self._transport.reconnect()

    def _request(self, command: str) -> str:
        response = self._transport.request(command)
        if response.startswith("ERR "):
            raise ControllerCommandError(response)
        if not response.startswith("OK "):
            raise TransportError(f"malformed controller response: {response}")
        return response

    def capabilities(self) -> DriverCapabilities:
        fields = parse_fields(self._request("MOTOR IDENTIFY"))
        return DriverCapabilities(
            fields.get("AZ_PRESENT") == "1",
            fields.get("EL_PRESENT") == "1",
            fields.get("UART") == "1",
        )

    def axis_configuration(self, axis: str) -> AxisConfiguration:
        axis_name = _axis(axis, allow_both=False)
        fields = parse_fields(self._request(f"MOTOR CONFIG {axis_name}"))
        return AxisConfiguration(
            axis_name,
            int(fields["FULL_STEPS"]),
            int(fields["MICROSTEPS"]),
            float(fields["GEAR_RATIO"]),
            float(fields["MIN_DEG"]),
            float(fields["MAX_DEG"]),
            float(fields["MAX_SPEED"]),
            float(fields["ACCEL"]),
            int(fields["RMS_MA"]),
            int(fields["HOLD_PERCENT"]),
            float(fields["HOME_OFFSET"]),
        )

    def axis_state(self, axis: str) -> AxisState:
        axis_name = _axis(axis, allow_both=False)
        fields = parse_fields(self._request(f"MOTOR STATUS {axis_name}"))
        return AxisState(
            axis_name,
            float(fields["DEG"]),
            int(fields["STEPS"]),
            int(fields["TARGET_STEPS"]),
            fields["TRUSTED"] == "1",
            fields["TRUST_LOSS"],
            fields["HOMED"] == "1",
            fields["MOVING"] == "1",
            fields["ENABLED"] == "1",
            fields["HOME_ACTIVE"] == "1",
            fields["HOMING"],
            fields["FAULT"],
            int(fields["LAST_COMMAND"]),
        )

    def diagnostics(self, axis: str) -> DriverDiagnostics:
        axis_name = _axis(axis, allow_both=False)
        fields = parse_fields(self._request(f"MOTOR DIAGNOSTICS {axis_name}"))
        return DriverDiagnostics(
            axis_name,
            fields["CONNECTED"] == "1",
            fields["ENABLED"] == "1",
            fields["FAULT"],
            fields["OTPW"] == "1",
            fields["OT"] == "1",
            fields["UV"] == "1",
            fields["RESET"] == "1",
            fields["OLA"] == "1",
            fields["OLB"] == "1",
            int(fields["CURRENT_SCALE"]),
        )

    def home_axis(self, axis: str = "BOTH") -> PositionReport:
        response = self._request(f"HOME {_axis(axis)}")
        self._wait_for_motion(response)
        return self.position()

    def home(self) -> PositionReport:
        return self.home_axis("BOTH")

    def move_relative(self, axis: str, delta_deg: float, speed_deg_per_s: float) -> PositionReport:
        _finite_positive(speed_deg_per_s, "speed_deg_per_s")
        if not isfinite(delta_deg):
            raise ValueError("delta_deg must be finite")
        response = self._request(f"MOVE_REL {_axis(axis)} {delta_deg:.9g} {speed_deg_per_s:.9g}")
        self._wait_for_motion(response)
        return self.position()

    def move_to(
        self,
        azimuth_deg: float,
        elevation_deg: float,
        speed_deg_per_s: float | None = None,
    ) -> PositionReport:
        speed = 10.0 if speed_deg_per_s is None else speed_deg_per_s
        _finite_positive(speed, "speed_deg_per_s")
        if not isfinite(azimuth_deg) or not isfinite(elevation_deg):
            raise ValueError("target angles must be finite")
        response = self._request(f"MOVE {azimuth_deg:.9g} {elevation_deg:.9g} {speed:.9g}")
        self._wait_for_motion(response)
        return self.position()

    def stop(self) -> None:
        self._request("STOP")

    def stop_axis(self, axis: str) -> None:
        self._request(f"MOTOR STOP {_axis(axis, allow_both=False)}")

    def emergency_stop(self) -> None:
        self._request("E_STOP")

    def reset_fault(self) -> None:
        self._request("CLEAR_FAULT")

    def position(self) -> PositionReport:
        response = self._request("STATUS")
        fields = parse_fields(response)
        trusted = fields["AZ_TRUSTED"] == "1" and fields["EL_TRUSTED"] == "1"
        moving = "MOVING=1" in response
        warnings = () if trusted else ("controller position is untrusted",)
        return PositionReport(
            float(fields["AZ_DEG"]),
            float(fields["EL_DEG"]),
            datetime.now(UTC),
            PositionKind.COMMANDED,
            PositionConfidence.TRUSTED if trusted else PositionConfidence.UNTRUSTED,
            motion_complete=trusted and not moving,
            warnings=warnings,
        )

    def _wait_for_motion(self, response: str) -> None:
        if "ACCEPTED=1" not in response and "READY=0" not in response:
            return
        command_id = parse_fields(response).get("ID")
        if command_id is None:
            raise ResponseCorrelationError("accepted motion response has no command ID")
        deadline = monotonic() + self._motion_timeout_s
        while monotonic() < deadline:
            event = self._transport.read_event(min(self._event_poll_s, deadline - monotonic()))
            if event is None:
                self._request("HEARTBEAT")
                continue
            fields = parse_fields(event)
            if event.startswith("EVENT FAULT "):
                raise ControllerCommandError("ERR " + fields.get("CODE", "UNKNOWN"))
            if event.startswith("EVENT ESTOP ") and fields.get("ACTIVE") == "1":
                raise ControllerCommandError("ERR EMERGENCY_STOP")
            if event.startswith("EVENT MOTION_COMPLETE "):
                event_id = fields.get("ID")
                if event_id == command_id:
                    return
                if event_id is not None and int(event_id) < int(command_id):
                    continue
                raise ResponseCorrelationError(
                    f"motion event ID {event_id or 'none'} did not match {command_id}"
                )
        self.stop()
        raise TransportTimeout(f"motion command {command_id} timed out")


def _axis(axis: str, *, allow_both: bool = True) -> str:
    normalized = axis.upper()
    allowed = {"AZ", "EL", "BOTH"} if allow_both else {"AZ", "EL"}
    if normalized not in allowed:
        raise ValueError(f"axis must be one of {', '.join(sorted(allowed))}")
    return normalized


def _finite_positive(value: float, name: str) -> None:
    if not isfinite(value) or value <= 0.0:
        raise ValueError(f"{name} must be finite and positive")
