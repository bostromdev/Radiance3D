from __future__ import annotations

from collections import deque
from typing import Any

import pytest

from radiance3d.transport import (
    DeviceIdentityError,
    ResponseCorrelationError,
    SerialTransport,
    TransportTimeout,
)


class FakeSerial:
    def __init__(self, responses: list[bytes], **_: Any) -> None:
        self.responses = deque(responses)
        self.writes: list[bytes] = []
        self.is_open = True
        self.reset_count = 0

    def reset_input_buffer(self) -> None:
        self.reset_count += 1

    def write(self, value: bytes) -> int:
        self.writes.append(value)
        return len(value)

    def flush(self) -> None:
        pass

    def readline(self) -> bytes:
        return self.responses.popleft() if self.responses else b""

    def close(self) -> None:
        self.is_open = False


def factory_for(*instances: FakeSerial) -> Any:
    remaining = deque(instances)

    def factory(**_: Any) -> FakeSerial:
        return remaining.popleft()

    return factory


def test_serial_validates_identity_correlates_commands_and_disconnects() -> None:
    serial = FakeSerial(
        [
            b"OK IDENTIFY DEVICE=Radiance3D CONTROLLER=motion PROTOCOL=1 MODE=PHYSICAL\n",
            b"EVENT FAULT CODE=NONE\n",
            b"OK ID=1 MOTOR_IDENTIFY AZ_PRESENT=1 EL_PRESENT=1 UART=1\n",
        ]
    )
    transport = SerialTransport("/dev/test-controller", serial_factory=factory_for(serial))

    transport.connect()
    response = transport.request("MOTOR IDENTIFY")

    assert "ID=1" in response
    assert transport.read_event(0.0) == "EVENT FAULT CODE=NONE"
    assert serial.writes == [b"IDENTIFY\n", b"CMD 1 MOTOR IDENTIFY\n"]
    transport.disconnect()
    assert not transport.connected
    assert not serial.is_open


def test_serial_rejects_wrong_identity_and_stale_response() -> None:
    wrong_device = FakeSerial([b"OK IDENTIFY DEVICE=Other PROTOCOL=1\n"])
    transport = SerialTransport("/dev/wrong", serial_factory=factory_for(wrong_device))
    with pytest.raises(DeviceIdentityError):
        transport.connect()

    stale = FakeSerial(
        [
            b"OK IDENTIFY DEVICE=Radiance3D PROTOCOL=1\n",
            b"OK ID=0 STATUS\n",
        ]
    )
    transport = SerialTransport("/dev/stale", serial_factory=factory_for(stale))
    with pytest.raises(ResponseCorrelationError, match="expected response ID 1"):
        transport.request("STATUS")


def test_serial_timeout_and_explicit_reconnect() -> None:
    timed_out = FakeSerial([b"OK IDENTIFY DEVICE=Radiance3D PROTOCOL=1\n"])
    replacement = FakeSerial([b"OK IDENTIFY DEVICE=Radiance3D PROTOCOL=1\n"])
    transport = SerialTransport(
        "/dev/reconnect",
        read_timeout_s=0.001,
        connection_timeout_s=0.001,
        serial_factory=factory_for(timed_out, replacement),
    )

    transport.connect()
    with pytest.raises(TransportTimeout):
        transport.request("STATUS", timeout_s=0.001)
    transport.reconnect()

    assert not timed_out.is_open
    assert replacement.is_open
