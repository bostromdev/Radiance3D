# Motion protocol version 1

Version 1 is UTF-8/ASCII text, one command or response per line, at a provisionally
documented 115200 baud. Whitespace separates fields. Angles are decimal degrees,
rates are degrees per second, and future time fields will use integer milliseconds.

## Commands

| Command | Arguments | Success response | Purpose |
| --- | --- | --- | --- |
| `IDENTIFY` | none | `OK IDENTIFY DEVICE=… PROTOCOL=1 MODE=…` | Identify device and protocol. |
| `STATUS` | none | `OK STATUS …` | Report positions, homing, stop, and fault state. |
| `POSITION` | none | `OK STATUS …` | Report current position state. |
| `HOME` | `AZ`, `EL`, or `BOTH` | `OK HOME AXIS=…` | Establish an axis zero. |
| `MOVE` | `AZ_DEG EL_DEG DEG_PER_S` | `OK MOVE …` | Move to an absolute position. |
| `SCAN_STEP` | `AZ_DEG EL_DEG DEG_PER_S` | `OK SCAN_STEP … READY=1` | Move and signal a measurement boundary. |
| `STOP` | none | `OK STOP` | Latch the controller in a stopped state. |
| `E_STOP` | none | `OK E_STOP` | Simulate activation of the dedicated emergency-stop input. |
| `ENABLE` | `0` or `1` | `OK ENABLE VALUE=…` | Disable or enable both driver outputs. |
| `CLEAR_FAULT` | none | `OK CLEAR_FAULT` | Clear a releasable fault/stop latch; never restore position confidence. |

Errors use `ERR CODE detail`. Defined simulator codes are `INVALID_COMMAND`,
`INVALID_ARGUMENT`, `INVALID_CONFIGURATION`, `NOT_HOMED`, `POSITION_UNTRUSTED`,
`LIMIT_REACHED`, `MOTION_TIMEOUT`, `DRIVER_DISABLED`, `EMERGENCY_STOP`, and
`STOPPED`.

`STATUS` labels `AZ_DEG` and `EL_DEG` with `POSITION_KIND=COMMANDED` and reports
per-axis homed/trusted state, driver enable, stop, emergency stop, and fault. Startup
is untrusted. Reset, stop, emergency stop, driver disable, timeout, or suspected missed
steps requires a new home operation; `CLEAR_FAULT` alone is insufficient.

## Important limitations

`READY=1 POSITION_KIND=COMMANDED` means the simulator completed its immediate state
update. It does not establish physical settling or verify position. The host still
applies its configured settling delay. A physical implementation must read/debounce
home inputs, apply acceleration and timeout, report active limits, define command IDs
or acknowledgements if needed, and preserve safe stop behavior across communication
loss. Emergency-stop release is a physical input condition, not a software clear.
