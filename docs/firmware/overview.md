# Firmware overview

The ESP32 firmware now has physical and simulated implementations behind the same
`MotionController` and `StepperDriver` boundaries. The physical path consists of an
Arduino ESP32 platform adapter, two TMC2209 UART/STEP/DIR drivers, one reusable
non-blocking axis controller per motor, a dual-axis coordinator, and protocol engine.

The controller uses integer microsteps as authoritative position. It services STEP
edges, acceleration, switch debounce, homing, driver diagnostics, emergency stop, and
serial input without long delay loops. A coordinated command completes only after
both axes stop. Critical faults stop both axes when a coordinated move is active.

The TMC2209 implementation is a small, datasheet-based register driver rather than a
third-party motion library. This keeps timer ownership, stop behavior, dual-axis
servicing, and native tests explicit. It supports addressed UART checks, IFCNT write
verification, RMS current and hold-current configuration, microsteps, interpolation,
stealthChop/spreadCycle selection, and diagnostic mapping.

The simulator models the public state/fault contract, not electrical waveforms.
The `esp32dev` environment compiles the physical implementation, but its board and
pin map remain provisional until the exact board and carrier modules are inspected.
See the [commissioning guide](../hardware/tmc2209-commissioning.md).
