"""End-to-end protocol regression against the native C++ simulator binary."""

from __future__ import annotations

import os
import select
import subprocess
import time
from collections.abc import Callable
from pathlib import Path
from typing import Any

import pytest

from radiance3d.motion_client import PhysicalMotionController
from radiance3d.transport import SerialTransport


class SimulatorSerial:
    """Small pyserial-shaped adapter around the native simulator process."""

    def __init__(self, executable: Path, *, timeout: float) -> None:
        self.timeout = timeout
        self._process: subprocess.Popen[bytes] = subprocess.Popen(
            [str(executable)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

    @property
    def is_open(self) -> bool:
        return self._process.poll() is None

    def reset_input_buffer(self) -> None:
        assert self._process.stdout is not None
        while select.select([self._process.stdout], [], [], 0.0)[0]:
            self._process.stdout.readline()

    def write(self, data: bytes) -> int:
        assert self._process.stdin is not None
        self._process.stdin.write(data)
        return len(data)

    def flush(self) -> None:
        assert self._process.stdin is not None
        self._process.stdin.flush()

    def readline(self) -> bytes:
        assert self._process.stdout is not None
        readable, _, _ = select.select([self._process.stdout], [], [], self.timeout)
        return self._process.stdout.readline() if readable else b""

    def close(self) -> None:
        if self.is_open:
            self._process.terminate()
            try:
                self._process.wait(timeout=1.0)
            except subprocess.TimeoutExpired:
                self._process.kill()
                self._process.wait(timeout=1.0)


@pytest.fixture
def simulator_factory() -> Callable[..., SimulatorSerial]:
    configured_path = os.environ.get("RADIANCE3D_SIMULATOR")
    if configured_path is None:
        pytest.skip("native firmware simulator is not configured")
    executable = Path(configured_path)
    if not executable.is_file():
        pytest.skip(f"native firmware simulator is missing: {executable}")

    def factory(**kwargs: Any) -> SimulatorSerial:
        timeout = kwargs.get("timeout")
        assert isinstance(timeout, float)
        return SimulatorSerial(executable, timeout=timeout)

    return factory


def test_python_host_client_remains_compatible_with_native_simulator(
    simulator_factory: Callable[..., SimulatorSerial],
) -> None:
    transport = SerialTransport("native-simulator", serial_factory=simulator_factory)
    client = PhysicalMotionController(transport)

    client.connect()
    assert client.capabilities().uart_diagnostics
    assert client.axis_configuration("AZ").microsteps == 16
    assert client.home().confidence.value == "trusted"
    position = client.move_to(18.0, 9.0, 5.0)
    assert position.azimuth_deg == 18.0
    assert position.elevation_deg == 9.0
    client.disconnect()


def test_native_simulator_enforces_the_v1_host_heartbeat(
    simulator_factory: Callable[..., SimulatorSerial],
) -> None:
    transport = SerialTransport("native-simulator", serial_factory=simulator_factory)
    transport.connect()
    assert transport.request("HOME BOTH").startswith("OK ID=1 HOME")

    time.sleep(2.1)
    assert transport.read_event(0.5) == (
        "EVENT FAULT CODE=DRIVER_DISABLED DETAIL=HOST_HEARTBEAT_TIMEOUT"
    )
    transport.disconnect()
