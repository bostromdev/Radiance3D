# Hardware overview

The Version 1 baseline consists of an ESP32 development board, two TMC2209 target
drivers behind a replaceable motion interface, two NEMA 17 bipolar steppers, one home
switch per axis, an emergency-stop input, a pan-tilt AUT fixture, one protected 12 V
input, and regulated ESP32 power. The RF measurement device is independently connected
to the host and remains stationary with the RF source/reference.

The architecture is fixed; exact board, modules, motors, currents, mechanics, pinout,
power rating, and RF device remain provisional. Component families are not tested
recommendations. Electrical, mechanical, thermal, and RF validation must precede a
supported configuration. See the [Version 1 baseline](../architecture/version-1.md).
