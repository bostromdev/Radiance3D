# ESP-IDF migration inventory

**Status:** implementation inventory recorded before the native ESP-IDF
migration.  The physical hardware remains unvalidated.

## Baseline

- Source branch: `codex/tmc2209-motion-control` at `a954db8`.
- Migration branch: `codex/esp-idf-migration`.
- Existing public protocol: line-oriented ASCII protocol version 1 at 115200
  baud, including `CMD` correlation, `EVENT` messages, heartbeat behavior,
  and simulator mode.
- Existing physical target: PlatformIO `esp32dev` using Arduino-ESP32.

## Arduino dependency inventory and replacement map

| Existing dependency | Location | Native ESP-IDF replacement |
| --- | --- | --- |
| `Arduino.h` and `#ifdef ARDUINO` | `src/main.cpp`, `include/esp32_platform.hpp` | Remove; use a physical `app_main()` and a separate host simulator executable. |
| `setup()` / `loop()` | `src/main.cpp` | `extern "C" void app_main(void)` plus bounded FreeRTOS protocol, motion, safety, and diagnostics tasks. |
| `pinMode`, `digitalWrite`, `digitalRead` | `src/esp32_platform.cpp` | Centralized `gpio_config`, `gpio_set_level`, and `gpio_get_level` in `IdfHardwarePlatform`. |
| `Serial` for the host protocol | `src/main.cpp` | UART0 ESP-IDF driver, owned only by the protocol task. |
| `HardwareSerial`, `Serial1`, `Serial2`, `SERIAL_8N1` | `src/esp32_platform.cpp` | `uart_param_config`, `uart_set_pin`, `uart_driver_install`, `uart_write_bytes`, and `uart_read_bytes`. |
| `millis()` heartbeat clock | `src/main.cpp` | `esp_timer_get_time()` in the safety task. |
| `yield()` UART polling | `src/esp32_platform.cpp` | bounded blocking UART reads in a FreeRTOS task. |
| loop-serviced STEP edges | `src/axis_controller.cpp` | two one-shot GPTimer edge schedulers; ISR work is restricted to GPIO edge emission and notification. |
| PlatformIO Arduino target and CI | `platformio.ini`, `.github/workflows/firmware.yml` | ESP-IDF CMake project, `sdkconfig.defaults`, Kconfig, CTest portable tests, and pinned ESP-IDF CI. |

There are no third-party Arduino libraries, Arduino interrupt APIs, Arduino
watchdog APIs, EEPROM/Preferences use, or Arduino timer libraries.  The
TMC2209 driver is already custom C++ register/CRC code and stays
driver-neutral behind `StepperDriver`.

## Migration-sensitive behavior

- The current core is single-threaded.  Native tasks must use queues and a
  single motion-state owner; they must not concurrently mutate
  `PhysicalMotionController`, `AxisController`, or `Tmc2209Driver`.
- The current step service combines pulse edges with homing debounce and
  occasional blocking TMC reads.  Diagnostics and UART reads cannot execute
  in a timer callback.
- UART0 is the USB host protocol and UART1/UART2 are assigned to the two
  TMC2209 devices.  ESP-IDF logs must be suppressed or redirected so they
  never corrupt the host protocol stream.
- TMC PDN_UART uses a single-wire electrical model.  The IDF adapter must
  tolerate write echo, bound reads, validate CRC, and retain IFCNT write
  verification.
- GPIO home and emergency inputs must perform only notification in ISR
  context.  Debounce and latching remain task work.

## Configuration findings

The profile at `firmware/config/provisional-esp32dev-v1.json` and compiled
defaults in `src/hardware_config.cpp` duplicate the same provisional hardware
configuration.  The native build will generate one validated configuration
header from the JSON profile and test it for consistency.  The request says
the elevation hold current is 40%, while the source profile and current
firmware say 30%.  The migration preserves the existing documented 30%
baseline until that physical-hardware decision is separately validated.

The current public `GEAR_RATIO` is a decimal value and the profile is `1.0`.
No unvalidated gear-ratio change is part of this migration; portable
conversion code remains host-testable.

## Timing decision

The documented maximum pulse rates are below 90 steps/second per axis.  Two
independent one-shot GPTimer schedulers are the simplest native mechanism for
the required two-microsecond pulse and direction setup timing while retaining
immediate shutdown.  This is an architecture decision, not a claim of
measured timing accuracy; logic-analyzer validation remains required.

## Validation constraints at inventory time

No usable `idf.py` or PlatformIO executable is installed locally.  The native
project will pin ESP-IDF v5.5.4 in documentation and CI.  Portable CMake/CTest
and Python checks can run locally; ESP-IDF compile, target component tests, and
hardware tests require a provisioned ESP-IDF environment and target hardware.
