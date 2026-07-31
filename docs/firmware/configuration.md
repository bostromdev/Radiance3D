# Firmware configuration

Configuration must eventually separate:

- controller and board definition;
- azimuth and elevation driver type and pins;
- steps, gearing, and microsteps per degree;
- direction, speed, acceleration, and machine limits;
- homing direction, switch polarity, and timeouts;
- communication rate and protocol version; and
- simulator versus physical mode.

No default pinout or motor calibration is supplied because none is verified. Firmware
build constants are not substitutes for wiring records or mechanical limit testing.
