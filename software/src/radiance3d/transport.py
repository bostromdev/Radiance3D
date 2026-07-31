"""Transport-independent request/response framing and optional serial I/O."""

from __future__ import annotations

from collections import deque
from collections.abc import Callable
from importlib import import_module
from time import monotonic
from typing import Any, Protocol, cast, runtime_checkable


class TransportError(RuntimeError):
    """Base error for controller transport failures."""


class TransportTimeout(TransportError):
    """Raised when a complete controller response is not received in time."""


class DeviceIdentityError(TransportError):
    """Raised when the connected serial device is not a compatible controller."""


class ResponseCorrelationError(TransportError):
    """Raised when a response does not match the outstanding command."""


@runtime_checkable
class ProtocolTransport(Protocol):
    """Minimal command/event boundary implemented by serial and simulators."""

    @property
    def connected(self) -> bool:
        """Whether the underlying device is open and validated."""

    def connect(self) -> None:
        """Open and validate the device."""

    def disconnect(self) -> None:
        """Close the device and release resources."""

    def reconnect(self) -> None:
        """Close and reopen the configured device."""

    def request(self, command: str, timeout_s: float | None = None) -> str:
        """Send one correlated command and return its response."""

    def read_event(self, timeout_s: float) -> str | None:
        """Return one asynchronous event, or None at the timeout."""


SerialFactory = Callable[..., Any]


class SerialTransport:
    """Line-oriented USB serial adapter for the Radiance3D protocol."""

    def __init__(
        self,
        port: str,
        *,
        baudrate: int = 115_200,
        read_timeout_s: float = 0.25,
        connection_timeout_s: float = 2.0,
        expected_protocol: str = "1",
        serial_factory: SerialFactory | None = None,
    ) -> None:
        if not port.strip():
            raise ValueError("a serial device path must be supplied")
        if baudrate <= 0 or read_timeout_s <= 0.0 or connection_timeout_s <= 0.0:
            raise ValueError("serial baud rate and timeouts must be positive")
        self._port = port
        self._baudrate = baudrate
        self._read_timeout_s = read_timeout_s
        self._connection_timeout_s = connection_timeout_s
        self._expected_protocol = expected_protocol
        self._serial_factory = serial_factory
        self._serial: Any | None = None
        self._command_id = 0
        self._events: deque[str] = deque()

    @property
    def connected(self) -> bool:
        return bool(self._serial is not None and getattr(self._serial, "is_open", True))

    def _factory(self) -> SerialFactory:
        if self._serial_factory is not None:
            return self._serial_factory
        try:
            serial_module = import_module("serial")
        except ImportError as error:
            raise TransportError(
                "serial support requires the 'radiance3d[serial]' extra"
            ) from error
        return cast(SerialFactory, serial_module.Serial)

    def connect(self) -> None:
        if self.connected:
            return
        try:
            self._serial = self._factory()(
                port=self._port,
                baudrate=self._baudrate,
                timeout=self._read_timeout_s,
                write_timeout=self._connection_timeout_s,
            )
            reset = getattr(self._serial, "reset_input_buffer", None)
            if reset is not None:
                reset()
            identity = self._raw_request("IDENTIFY", self._connection_timeout_s)
        except Exception as error:
            self.disconnect()
            if isinstance(error, TransportError):
                raise
            raise TransportError(f"could not connect to {self._port}: {error}") from error
        fields = parse_fields(identity)
        if (
            not identity.startswith("OK IDENTIFY ")
            or fields.get("DEVICE") != "Radiance3D"
            or fields.get("PROTOCOL") != self._expected_protocol
        ):
            self.disconnect()
            raise DeviceIdentityError(
                "serial device did not identify as the expected Radiance3D protocol"
            )

    def disconnect(self) -> None:
        serial_port, self._serial = self._serial, None
        self._events.clear()
        if serial_port is not None:
            try:
                serial_port.close()
            except Exception as error:
                raise TransportError(f"could not close serial device: {error}") from error

    def reconnect(self) -> None:
        self.disconnect()
        self.connect()

    def _write_line(self, line: str) -> None:
        if not self.connected:
            raise TransportError("serial device is not connected")
        assert self._serial is not None
        try:
            self._serial.write((line + "\n").encode("ascii"))
            flush = getattr(self._serial, "flush", None)
            if flush is not None:
                flush()
        except Exception as error:
            raise TransportError(f"serial write failed: {error}") from error

    def _read_line(self, deadline: float) -> str:
        assert self._serial is not None
        while monotonic() < deadline:
            try:
                raw = cast(bytes, self._serial.readline())
            except Exception as error:
                raise TransportError(f"serial read failed: {error}") from error
            if not raw:
                continue
            try:
                line = raw.decode("ascii").strip()
            except UnicodeDecodeError as error:
                raise TransportError("controller returned non-ASCII data") from error
            if line:
                return line
        raise TransportTimeout("controller response timed out")

    def _raw_request(self, command: str, timeout_s: float) -> str:
        self._write_line(command)
        deadline = monotonic() + timeout_s
        while True:
            line = self._read_line(deadline)
            if line.startswith("EVENT "):
                self._events.append(line)
                continue
            return line

    def request(self, command: str, timeout_s: float | None = None) -> str:
        if not command.strip() or "\n" in command or "\r" in command:
            raise ValueError("command must be one non-empty line")
        if not self.connected:
            self.connect()
        self._command_id += 1
        command_id = self._command_id
        response = self._raw_request(
            f"CMD {command_id} {command}", timeout_s or self._connection_timeout_s
        )
        response_id = parse_fields(response).get("ID")
        if response_id != str(command_id):
            raise ResponseCorrelationError(
                f"expected response ID {command_id}, received {response_id or 'none'}"
            )
        return response

    def read_event(self, timeout_s: float) -> str | None:
        if timeout_s < 0.0:
            raise ValueError("event timeout must not be negative")
        if self._events:
            return self._events.popleft()
        if not self.connected:
            raise TransportError("serial device is not connected")
        deadline = monotonic() + timeout_s
        while monotonic() < deadline:
            try:
                line = self._read_line(deadline)
            except TransportTimeout:
                return None
            if line.startswith("EVENT "):
                return line
            raise ResponseCorrelationError("received an unsolicited command response")
        return None


def parse_fields(line: str) -> dict[str, str]:
    """Parse the protocol's whitespace-separated KEY=VALUE fields."""

    fields: dict[str, str] = {}
    for token in line.split():
        if "=" in token:
            key, value = token.split("=", 1)
            fields[key] = value
    return fields
