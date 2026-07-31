# Motion controller foundation

The controller currently implements an in-memory simulator for protocol and host
integration work. It compiles as a native command-line program and as a provisional
ESP32 Arduino target. Neither build drives pins.

```bash
pio run -e native
```

The native process reads one command per line from standard input and writes one
response per line. See [the protocol specification](../../docs/firmware/protocol.md).
Board selection, electrical limits, motor drivers, pins, and emergency-stop behavior
remain provisional.
