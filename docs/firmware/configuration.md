# Firmware configuration

Version 1 separates board/pin configuration from driver-neutral motion behavior. The
implemented simulator uses typed controller configuration; a physical target must
populate the same fields from a versioned configuration record.

## Axis fields

Each azimuth and elevation axis defines:

- `motor_full_steps_per_revolution` (provisionally 200 for a typical 1.8° motor);
- `microsteps`;
- configurable `motor_rms_current_ma` (`0` means deliberately unset in the simulator;
  a physical driver must refuse operation until a safe value is selected);
- `gear_ratio`;
- calculated `steps_per_output_revolution`;
- direction inversion;
- home offset in degrees;
- minimum and maximum angle in degrees;
- maximum speed in degrees per second;
- acceleration in degrees per second squared; and
- switch normally-closed state, debounce milliseconds, homing direction, homing
  speed, back-off degrees, and second slow-approach speed.

The conversion is `full steps × microsteps × gear ratio`. It describes commands, not
measured mechanics. Backlash compensation may later use this configuration boundary,
but Version 1 does not implement it.

## Controller fields

Controller configuration contains motion timeout, emergency-stop active polarity,
protocol version, USB serial rate, board definition, physical pin mapping, and
simulator/physical mode. A TMC2209-specific physical configuration will additionally
record UART address/connection, sense-resistor/module revision, and safely selected RMS
motor current. Current remains unset until the exact motors and thermal design exist.

No default pinout or physical motor calibration is supplied because none is verified.
ESP32 signals are 3.3 V and no attached module is assumed 5 V tolerant. Firmware build
constants are not substitutes for a wiring record, thermal test, or mechanical-limit
test.
