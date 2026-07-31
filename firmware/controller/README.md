# Motion controller foundation

The controller currently implements an in-memory `MotionController` for protocol and
host integration work. It compiles as a native command-line program and as an ESP32
Arduino target for the Version 1 hardware baseline. The physical wiring, carrier
revision, and bring-up details remain pending validation.

```bash
pio run -e native
pio test -e native
```

The native process reads one command per line from standard input and writes one
response per line. See [the protocol specification](../../docs/firmware/protocol.md).
The simulator enforces configuration-derived limits and position-confidence rules.
Board selection, electrical limits, motor-driver carrier details, pins, physical
homing, and physical emergency-stop behavior remain pending validation against the
actual hardware.
