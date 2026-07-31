# Native ESP-IDF firmware architecture

The physical ESP32 controller uses native ESP-IDF v5.5.4. The source tree keeps
portable behavior under the existing C++ interfaces and places ESP-IDF-only adapters
under `firmware/controller/components/platform_idf`.

## Startup and safe state

`app_main()` initializes NVS, records the ESP32 reset reason, constructs the validated
hardware profile, drives both STEP pins low, makes both enable pins inactive, and then
initializes GPIO inputs, UART0 host transport, TMC UARTs, TMC probes, motion timers,
queues, event groups, ISRs, and tasks. Both axes begin disabled and position-untrusted.

Any startup failure leaves the enable outputs inactive and is reported as a structured
`EVENT STARTUP READY=0` line. `RESET=<reason>` is included in every startup event;
after every reset, homing is required before absolute motion.

## Task ownership

| Task | Priority | Stack | Ownership |
| --- | ---: | ---: | --- |
| Safety | 9 | 3072 | E-stop input supervision and host-heartbeat enforcement. |
| Motion | 8 | 6144 | The only owner of physical controller, axes, TMC UARTs, motion state, homing, and protocol engine. |
| Protocol | 5 | 4096 | UART0 framing and serialized host responses/events. |
| Diagnostics | 3 | 3072 | Periodically requests an idle-only diagnostic read through the motion owner; it never touches a driver directly. |

Tasks are intentionally not pinned to an ESP32 core. No measured reason currently
justifies affinity. Kconfig owns task stacks, watchdog duration, heartbeat duration,
and diagnostics interval; it does not duplicate the board profile or runtime tuning.

`QueueHandle_t` carries typed host commands, safety requests, and outbound lines.
`EventGroupHandle_t` exposes enabled/moving state to the safety task. Task notifications
wake the motion task on queue writes, GPIO input changes, and completed GPTimer pulses.
The motion task is the single writer of `PhysicalMotionController`, preventing UART and
axis-state races.

## STEP timing

Each axis uses a one-shot 1 MHz GPTimer scheduler. The timer callback performs only:

1. STEP high;
2. reschedule for a conservative five-microsecond pulse width;
3. STEP low, disarm, increment a completion counter, and notify the motion task.

Direction setup, acceleration/deceleration, integer position updates, homing,
timeouts, and all UART work stay in the motion task. TMC diagnostics run only while
both axes are idle, so a bounded UART timeout cannot delay arming a following pulse.
A scheduler can force STEP low from the e-stop ISR; the same cache-safe ISR also drives
both enable pins inactive, and the motion task latches fault/trust state. The design
preserves conservative high/low widths and direction setup but does **not**
claim measured pulse timing or jitter. Logic-analyzer validation is required.

GPTimer was selected over RMT/MCPWM/`esp_timer` because the profile's configured step
rates are below 90 steps/s per axis and two small one-shot schedulers are the simplest
testable design with independent axes and immediate disarm behavior.

## GPIO and emergency input

`IdfHardwarePlatform` centralizes GPIO configuration through `gpio_config`. The
generated profile is checked for duplicate pins, output-capability conflicts, and ESP32
boot-strapping pins before driver probing. The profile selects input pull-up, pull-down,
or no internal bias independently for home and e-stop inputs; profile polarity remains
configurable. Bootstrap-pin use is reported as a structured startup warning.

GPIO ISRs do the minimum cache-safe work. An active e-stop edge immediately pulls STEP
low, disables both drivers, and requests a motion-owner latch; input debounce and all
state/protocol work remain outside the ISR. Any asserted edge is intentionally
fail-safe-latched even if it later bounces, so timer and motion state cannot diverge.
Firmware e-stop supplements—rather than replaces—a physical motor-power disconnect.

## UART and logging

UART0 is exclusively the structured host protocol at the baud rate generated from the
hardware profile (115200 for Version 1). ESP-IDF console output is disabled in
`sdkconfig.defaults`, so `ESP_LOGx` cannot corrupt protocol frames. Boot ROM output
before the application starts may still be visible on some boards and must not be
treated as protocol data.

UART1 and UART2 are dedicated to the azimuth and elevation TMC2209 links. The adapter
uses normal ESP-IDF UART mode with bounded timeouts; PDN_UART one-wire behavior is an
external electrical topology, not ESP-IDF RS-485/RTS mode. TX must join PDN_UART through
the carrier-required resistor and RX must observe that same bus. The driver scans bounded
receive chunks for a CRC-valid reply so write echo cannot be mistaken for a reply and
verifies IFCNT around configuration writes. The exact carrier PDN_UART wiring and R10
value remain hardware-validation items.

Use tags `APP`, `CONFIG`, `PROTOCOL`, `MOTION`, `AXIS_AZ`, `AXIS_EL`, `TMC2209_AZ`,
`TMC2209_EL`, `SAFETY`, and `DIAGNOSTICS`. Current default logging is intentionally
quiet on the protocol board; use a debugger or a separately wired diagnostics transport
for detailed field logs.

## Watchdogs, reset, and brownout

Three mechanisms have different purposes:

| Mechanism | Purpose | Response |
| --- | --- | --- |
| ESP-IDF task watchdog | Detects stalled registered tasks. | Reset; next startup reports reset reason and begins disabled/untrusted. |
| Host heartbeat watchdog | Detects an absent host while a driver is enabled. | Stop, disable drivers, invalidate trust, emit `HOST_HEARTBEAT_TIMEOUT`. |
| Motion/homing/driver timeouts | Detects a motion, homing, or TMC communication failure. | Fault, disable affected hardware, invalidate trust. |
| Physical e-stop | Operator safety input. | Immediately pull STEP low and disable outputs in the ISR, then latch state and notify the host in task context. |

ESP32 brownout detection is enabled. A brownout restart is reported as `RESET=BROWNOUT`
and never restores position trust. The LM2596 display is not evidence of transient
behavior; power-drop and brownout tests remain pending.

## Configuration

`firmware/config/provisional-esp32dev-v1.json` is the authoritative Version 1 profile.
The build validates it and generates `hardware_profile_generated.hpp`, consumed by both
ESP-IDF and host CMake builds. Build settings that are not hardware profile values live
in Kconfig. NVS is initialized but no unsafe runtime motor configuration is persisted:
runtime current/speed settings remain bounded by the generated profile and reset to a
known safe baseline after reboot.
