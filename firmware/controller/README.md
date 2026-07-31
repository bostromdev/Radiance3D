# Motion controller foundation

The controller currently implements an in-memory `MotionController` for protocol and
host integration work. It compiles as a native command-line program and as a
provisional ESP32 Arduino target. Neither build drives pins or implements a TMC2209.

```bash
pio run -e native
pio test -e native
```

The native process reads one command per line from standard input and writes one
response per line. See [the protocol specification](../../docs/firmware/protocol.md).
The simulator enforces configuration-derived limits and position-confidence rules.
Board selection, electrical limits, motor drivers, pins, physical homing, and physical
emergency-stop behavior remain provisional.
