# ADR: Use native ESP-IDF for physical motion firmware

**Status:** Accepted for the physical ESP32 target.  Hardware validation remains
pending.

## Context

The Version 1 controller needs two independent STEP outputs, two TMC2209 UART
links, a structured USB serial protocol, non-blocking homing and motion, a latched
emergency input, a host heartbeat watchdog, reset inspection, and explicit fault
handling. The portable protocol and motion rules also need to remain executable on a
host simulator.

The previous physical target used an Arduino runtime through PlatformIO. Its
cooperative loop combined GPIO pulse generation, switch debounce, protocol I/O, and
driver UART diagnostics. That was sufficient for an early baseline, but it did not
make task ownership, watchdog behavior, or ESP32 peripheral configuration explicit.

## Decision

Use native ESP-IDF v5.5.4 for the physical ESP32 firmware. The physical entry point
is `app_main()`. ESP-IDF owns GPIO, UART, GPTimer, FreeRTOS queues/tasks/event groups,
task watchdog registration, reset-reason inspection, NVS initialization, brownout
configuration, and logging.

Portable C++ remains separate from ESP-IDF adapters:

- `motion_controller` and `protocol` remain host-testable core behavior.
- `axis_controller`, `physical_motion_controller`, and `tmc2209_driver` remain
  driver-neutral C++ behind small platform interfaces.
- `components/platform_idf` supplies GPIO/UART/GPTimer implementations.
- `main` owns task setup and task-to-task messages.

The physical project is built with `idf.py`; a separate host CMake project builds the
simulator and portable tests. Arduino is not an ESP-IDF component and is not a
dependency of the physical target.

## Alternatives considered

| Alternative | Decision |
| --- | --- |
| Remain on Arduino-ESP32 | Rejected because the native task, watchdog, timer, and UART ownership would remain implicit. |
| Use Arduino as an ESP-IDF component | Rejected because it would retain the runtime dependency this migration removes. |
| Retain PlatformIO with Arduino | Rejected as the physical source of truth; it does not satisfy the native-runtime requirement. |
| Rust | Deferred; this controlled migration preserves the existing tested C++ core. |
| MicroPython | Rejected for this deterministic-control and driver-UART use case. |

## Consequences

Advantages include official vendor APIs, explicit resource ownership, GPTimer-backed
STEP edges, clearer host-protocol isolation, and a maintainable long-term embedded
architecture. Costs include a more involved setup, more verbose APIs, a new CMake
layout, and required physical revalidation.

Native ESP-IDF is not automatically better for every ESP32 application. This decision
is specific to the controller's timing, safety, and peripheral requirements. It does
not establish measured timing, motor operation, or reliability claims.
