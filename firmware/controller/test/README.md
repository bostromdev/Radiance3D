# Firmware tests

PlatformIO native tests exercise configuration-derived angular conversion, configured
travel limits, homing, position confidence, driver-disable behavior, and the protocol
synchronization boundary. Run them with:

```bash
pio test -e native
```

These tests exercise the simulator only. Physical hardware tests must state board
revision, wiring, load, supply, and safety controls.
