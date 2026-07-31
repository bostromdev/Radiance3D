# Motion protocol version 1

Version 1 is line-oriented ASCII at 115200 baud. Angles are decimal degrees, angular
rates are degrees per second, current is RMS milliamps, and integer positions are
microsteps at the configured motor/gear ratio.

## Correlation and events

Physical host traffic uses `CMD <positive-uint32-id> <command>`. Every direct response
contains the same `ID=<id>`. IDs must increase; reused and older IDs return
`DUPLICATE_COMMAND` and `STALE_COMMAND`. Uncorrelated commands remain supported for
interactive use and the native simulator.

Accepted physical motion can return before it completes. `ACCEPTED=1` or `READY=0`
means motion is in progress. The measurement boundary is:

```text
EVENT MOTION_COMPLETE ID=<id> AZ_DONE=1 EL_DONE=1
```

The controller also emits `EVENT FAULT CODE=<fault>` and
`EVENT ESTOP ACTIVE=<0|1>`. A host must not measure until the completion ID matches
the accepted scan/move command and both axes are done. It then applies the configured
settling delay.

## General commands

| Command | Arguments | Purpose |
| --- | --- | --- |
| `IDENTIFY` | none | Report device, protocol, physical/simulator mode. |
| `HEARTBEAT` | none | Keep the physical controller's two-second host watchdog alive. |
| `STATUS` / `POSITION` | none | Report commanded angles, integer steps, targets, homing, trust/reason, switches, enabled/moving state, last command, e-stop, and faults. |
| `HOME` | `AZ`, `EL`, or `BOTH` | Start the two-pass homing state machine. |
| `MOVE` | `AZ_DEG EL_DEG DEG_PER_S` | Start a homed absolute move. |
| `MOVE_REL` | `AXIS DELTA_DEG DEG_PER_S` | Start a homed relative move. |
| `SCAN_STEP` | `AZ_DEG EL_DEG DEG_PER_S` | Start a coordinated scan move; readiness comes from the completion event. |
| `STOP` | none | Stop both axes; an interrupted open-loop position becomes untrusted. |
| `E_STOP` | none | Latch software emergency stop and disable both drivers. |
| `ENABLE` | `0` or `1` | Change both driver-enable outputs. |
| `CLEAR_FAULT` / `RESET_ESTOP` | none | Clear releasable faults; reset is rejected while the physical e-stop input remains active. |

## Motor commands

| Command | Arguments | Purpose |
| --- | --- | --- |
| `MOTOR IDENTIFY` | none | Report driver presence and optional UART capability. |
| `MOTOR CONFIG` | `AZ` or `EL` | Inspect transport-neutral axis motion configuration. |
| `MOTOR STATUS` | `AZ` or `EL` | Inspect axis position, trust, homing, and fault state. |
| `MOTOR DIAGNOSTICS` | `AZ` or `EL` | Read optional driver connection/current-scale/thermal/electrical diagnostics. |
| `MOTOR ENABLE` / `DISABLE` | `AZ` or `EL` | Control one axis. |
| `MOTOR STEP` | `AXIS SIGNED_STEPS` | Bounded, unhomed bench move; position remains explicitly untrusted. |
| `MOTOR MOVE_DEGREES` | `AXIS SIGNED_DEGREES` | Bounded relative bench move converted once to integer steps. |
| `MOTOR SET_CURRENT` | `AXIS RMS_MA` | Set current within the configured ceiling while stopped. |
| `MOTOR SET_MICROSTEPS` | `AXIS VALUE` | Set a supported power-of-two microstep value; invalidates position trust when changed. |
| `MOTOR STOP` | `AZ` or `EL` | Immediately stop one axis. |

## Fault and trust contract

Errors use `ERR ID=<id> CODE detail`. Faults include invalid command/configuration,
not homed, position untrusted, limits, motion timeout, disabled driver, emergency
stop, driver communication/critical faults, the exact homing failure classes,
unexpected home-switch activation, and controlled stop.

`AZ_DEG` and `EL_DEG` are always labeled `POSITION_KIND=COMMANDED`. They are not
encoder measurements. Reset, emergency stop, critical driver failure, timeout,
interrupted motion, driver disable, configuration changes affecting scale, and failed
homing make position untrusted. Clearing a fault does not restore trust; successful
homing does.

On the physical ESP32 target, two seconds without a command/heartbeat while a driver
is enabled stops motion, disables both drivers, and emits a host-timeout fault event.
The Python motion client sends heartbeats while waiting. This watchdog does not make
USB serial or software an emergency-rated control path.
