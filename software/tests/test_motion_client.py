from __future__ import annotations

from collections import deque

import pytest

from radiance3d.interfaces import PositionConfidence
from radiance3d.motion_client import ControllerCommandError, PhysicalMotionController
from radiance3d.transport import ResponseCorrelationError, TransportTimeout


class FakeTransport:
    def __init__(self) -> None:
        self._connected = False
        self.command_id = 0
        self.events: deque[str | None] = deque()
        self.commands: list[str] = []
        self.responses: dict[str, str] = {}

    @property
    def connected(self) -> bool:
        return self._connected

    def connect(self) -> None:
        self._connected = True

    def disconnect(self) -> None:
        self._connected = False

    def reconnect(self) -> None:
        self.disconnect()
        self.connect()

    def request(self, command: str, timeout_s: float | None = None) -> str:
        del timeout_s
        self.command_id += 1
        self.commands.append(command)
        response = self.responses.get(command)
        if response is not None:
            return response.replace("{id}", str(self.command_id))
        return f"OK ID={self.command_id} ACCEPTED=1"

    def read_event(self, timeout_s: float) -> str | None:
        del timeout_s
        return self.events.popleft() if self.events else None


STATUS = (
    "OK ID={id} STATUS AZ_DEG=12.500 EL_DEG=-4.000 POSITION_KIND=COMMANDED "
    "AZ_HOMED=1 EL_HOMED=1 AZ_TRUSTED=1 EL_TRUSTED=1 "
    "AXIS=AZ MOVING=0 AXIS=EL MOVING=0 FAULT=NONE"
)


def controller() -> tuple[PhysicalMotionController, FakeTransport]:
    transport = FakeTransport()
    transport.responses["STATUS"] = STATUS
    return PhysicalMotionController(transport, motion_timeout_s=0.01, event_poll_s=0.001), transport


def test_host_reads_capabilities_configuration_state_and_diagnostics() -> None:
    motion, transport = controller()
    transport.responses["MOTOR IDENTIFY"] = (
        "OK ID={id} MOTOR_IDENTIFY AZ_PRESENT=1 EL_PRESENT=0 UART=1"
    )
    transport.responses["MOTOR CONFIG AZ"] = (
        "OK ID={id} MOTOR_CONFIG AXIS=AZ FULL_STEPS=200 MICROSTEPS=16 "
        "GEAR_RATIO=2.500 MIN_DEG=0.000 MAX_DEG=359.000 MAX_SPEED=10.000 "
        "ACCEL=20.000 RMS_MA=400 HOLD_PERCENT=30 HOME_OFFSET=1.500"
    )
    transport.responses["MOTOR STATUS AZ"] = (
        "OK ID={id} MOTOR_STATUS AXIS=AZ DEG=12.500 STEPS=222 TARGET_STEPS=222 "
        "POSITION_KIND=COMMANDED TRUSTED=1 TRUST_LOSS=NONE HOMED=1 MOVING=0 "
        "ENABLED=1 HOME_ACTIVE=0 HOMING=COMPLETE FAULT=NONE LAST_COMMAND=7"
    )
    transport.responses["MOTOR DIAGNOSTICS AZ"] = (
        "OK ID={id} MOTOR_DIAGNOSTICS AXIS=AZ CONNECTED=1 ENABLED=1 FAULT=NONE "
        "OTPW=1 OT=0 UV=0 RESET=0 OLA=0 OLB=1 CURRENT_SCALE=12"
    )

    assert motion.capabilities().azimuth_present
    assert not motion.capabilities().elevation_present
    assert motion.axis_configuration("az").gear_ratio == 2.5
    assert motion.axis_state("AZ").internal_steps == 222
    diagnostics = motion.diagnostics("az")
    assert diagnostics.overtemperature_warning
    assert diagnostics.open_load_b


def test_move_waits_for_matching_completion_and_returns_trusted_position() -> None:
    motion, transport = controller()
    transport.events.extend(
        [
            "EVENT MOTION_COMPLETE ID=0 AZ_DONE=1 EL_DONE=1",
            "EVENT MOTION_COMPLETE ID=1 AZ_DONE=1 EL_DONE=1",
        ]
    )

    report = motion.move_to(12.5, -4.0, 5.0)

    assert report.confidence is PositionConfidence.TRUSTED
    assert report.motion_complete
    assert transport.commands[:2] == ["MOVE 12.5 -4 5", "STATUS"]


def test_already_complete_motion_does_not_wait_for_an_event() -> None:
    motion, transport = controller()
    transport.responses["HOME BOTH"] = "OK ID={id} HOME AXIS=BOTH"

    report = motion.home()

    assert report.motion_complete
    assert transport.commands == ["HOME BOTH", "STATUS"]


def test_future_completion_and_driver_fault_are_not_silently_accepted() -> None:
    motion, transport = controller()
    transport.events.append("EVENT MOTION_COMPLETE ID=2 AZ_DONE=1 EL_DONE=1")
    with pytest.raises(ResponseCorrelationError):
        motion.move_relative("AZ", 1.0, 2.0)

    motion, transport = controller()
    transport.events.append("EVENT FAULT CODE=DRIVER_COMMUNICATION")
    with pytest.raises(ControllerCommandError) as error:
        motion.home()
    assert error.value.code == "DRIVER_COMMUNICATION"


def test_motion_timeout_sends_stop_and_reports_timeout() -> None:
    motion, transport = controller()

    with pytest.raises(TransportTimeout, match="motion command 1"):
        motion.move_relative("EL", -1.0, 2.0)

    assert transport.commands[-1] == "STOP"
