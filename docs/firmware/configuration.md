# Firmware configuration

Version 1 separates board/pin configuration from driver-neutral motion behavior.
The physical target and simulator use the same typed motion fields. The Version 1
hardware profile is stored in
[`firmware/config/provisional-esp32dev-v1.json`](../../firmware/config/provisional-esp32dev-v1.json);
the compiled defaults live in `hardware_config.cpp`.

## Axis fields

Each azimuth and elevation axis defines:

- `motor_full_steps_per_revolution` (200 for the selected 1.8° motor);
- `microsteps`;
- configurable `motor_rms_current_ma`;
- configurable maximum RMS-current ceiling and hold-current percentage;
- `gear_ratio`;
- calculated `steps_per_output_revolution`;
- direction inversion;
- home offset in degrees;
- minimum and maximum angle in degrees;
- maximum speed in degrees per second;
- acceleration, settling time, motion timeout, and bench-test step ceiling; and
- switch normally-closed state, debounce milliseconds, homing direction, homing
  speed, back-off degrees, and second slow-approach speed.

The conversion is `full steps × microsteps × gear ratio`. It describes commands, not
measured mechanics. Backlash compensation may later use this configuration boundary,
but Version 1 does not implement it.

## Controller fields

Controller configuration includes emergency-stop pin/polarity/debounce, protocol
version, serial rate, board name, and all GPIO assignments. Startup rejects duplicate
pins and input-only STEP/DIR/ENABLE/TX assignments and reports ESP32 bootstrapping-pin
use as a warning.

The example uses 650 mA RMS as the initial commissioning current with a 1000 mA
software ceiling. Before energizing a motor, verify this value against the selected
motor rating, the carrier's actual sense resistor and schematic, cooling, load, and
measured temperatures. ESP32 signals are 3.3 V and no attached module is assumed 5 V
tolerant. The example is not a verified pinout or motor calibration.
