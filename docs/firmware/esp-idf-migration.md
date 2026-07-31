# ESP-IDF migration notes

This migration changes the physical ESP32 runtime from Arduino-ESP32 to native
ESP-IDF without changing protocol version 1 or the Python motion API.

## What changed

- Physical startup is `app_main()` with FreeRTOS protocol, motion, safety, and
  diagnostics tasks.
- GPIO, UART, GPTimer, task watchdog, NVS initialization, reset reason, brownout
  configuration, and logging use ESP-IDF APIs.
- The host simulator is a separate portable CMake executable.
- `platformio.ini`, Arduino headers, Arduino serial/GPIO adapter, and the Arduino
  `setup()`/`loop()` entry point were removed.
- The board profile is validated and compiled from JSON rather than repeated in a
  handwritten C++ defaults table.
- Gear ratio is stored in the portable core as a rational number while retaining the
  exact decimal `GEAR_RATIO` protocol field.

## Compatibility

The protocol remains version 1. Device identity, command IDs, stale/duplicate
handling, line framing, heartbeat event wording, faults, trust semantics, simulator
mode, and host Python types are unchanged. Startup includes an additive reset-reason
field. Existing host parsers accept additive fields.

The native simulator now models the two-second host-heartbeat timeout in addition to
motion, homing, driver absence/thermal faults, e-stop commands, reset trust loss, and
protocol behavior. The CI path runs the unchanged Python serial client against the
compiled simulator.

## Validation boundary

Portable CTest and Python integration tests are evidence of compilation/simulation and
protocol compatibility. ESP-IDF build is evidence of target compilation. Neither is
evidence of measured pulse timing, motor motion, emergency-stop latency, UART wiring,
thermal behavior, or brownout behavior. See the [commissioning guide](../hardware/tmc2209-commissioning.md)
for the physical test sequence.
