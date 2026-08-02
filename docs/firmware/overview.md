# Firmware overview

The ESP32 physical firmware uses native ESP-IDF v5.5.4 and the host simulator uses a
separate portable CMake target. Both retain the same `MotionController`,
`StepperDriver`, and protocol boundaries. The physical path uses two TMC2209
UART/STEP/DIR drivers, reusable non-blocking axis controllers, a dual-axis coordinator,
and a protocol engine owned by the motion task.

Integer microsteps remain authoritative commanded position. GPTimer emits physical
STEP high/low edges; the motion task owns acceleration, switch debounce, homing,
diagnostics, timeouts, trust state, and dual-axis completion. Protocol, safety, and
diagnostics tasks communicate with it through FreeRTOS queues and notifications. A
critical coordinated-move fault stops both axes.

The TMC2209 implementation remains a small data-sheet-based register driver, not a
third-party motion library. It supports CRC framing, bounded UART timeouts, write-echo
filtering, IFCNT verification, RMS/hold current configuration, microsteps,
interpolation, stealthChop/spreadCycle selection, and diagnostic mapping.

The current profile remains a Version 1 provisional baseline. GPIO mapping, carrier
PDN_UART wiring, R10/sense resistance, current, pulse timing, e-stop latency, power,
and mechanical behavior require physical validation. See the
[native architecture](esp-idf-architecture.md) and
[commissioning guide](../hardware/tmc2209-commissioning.md).
