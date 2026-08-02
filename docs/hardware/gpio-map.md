# Final proposed GPIO map

## Verified Hardware

This is the sole firmware GPIO map, encoded in
`firmware/config/radiance3d-owned-hardware.json`. It uses only header labels visible
in the controller photo. GPIO21 and GPIO22 are reserved for the rear OLED and are not
available to the motion system. GPIO0, 2, 4, 5, 12, and 15 are avoided because they
can affect boot; flash-connected GPIO6–11 are never used.

| Function | GPIO | Direction | Harness | Status |
|---|---:|---|---|---|
| Pan STEP | 25 | output | SIG-001 | assigned |
| Pan DIR | 26 | output | SIG-001 | assigned |
| Pan ENN | 27 | output | SIG-001 | active-low, confirm carrier marking |
| Pan PDN UART TX/RX | 17 / 16 | output / input | SIG-001 | address 0 |
| Tilt STEP | 18 | output | SIG-002 | assigned |
| Tilt DIR | 19 | output | SIG-002 | assigned |
| Tilt ENN | 23 | output | SIG-002 | active-low, confirm carrier marking |
| Tilt PDN UART TX/RX | 14 / 39 | output / input-only | SIG-002 | address 1 |
| AD8317 analog output | 36 | ADC1 input-only | SIG-003 | silicone wire; ADC1; Wi-Fi-safe |
| E-stop input | 13 | input | not in owned BOM | reserved, uninstalled |
| OLED | 21 / 22 | reserved | board-integrated | bench-confirm actual rear OLED wiring |

No home switch is installed or assigned. The firmware rejects a homing request until
physical switches are added and documented; StallGuard is diagnostics only, not a
homing substitute. Firmware limits remain intentionally provisional and are not the
mechanical architecture specification. **Design Intent:** the mechanical system targets
one managed 360° pan turn from cable neutral; final limits require CAD
and physical testing.

GPIO39 is input-only and is used only as UART RX. GPIO36 is input-only and is used only
as ADC1. No assigned output uses an input-only pin, and no function shares a pin.
