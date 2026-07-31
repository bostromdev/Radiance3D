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
| `CLEAR_FAULT` | none | `OK CLEAR_FAULT` | Clear the simulator fault/stop latch. |

Errors use `ERR CODE detail`. Defined simulator codes are `INVALID_COMMAND`,
`INVALID_ARGUMENT`, `NOT_HOMED`, `LIMIT_REACHED`, and `STOPPED`.

## Important limitations

`READY=1` means the simulator updated its state. It does not establish physical
settling. A physical implementation must distinguish commanded and observed position,
report active limits, define command IDs or acknowledgements if needed, and preserve
stop behavior across communication loss where safety analysis requires it.
