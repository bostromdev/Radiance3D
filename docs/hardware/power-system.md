# Power system

Version 1 uses one 12 V DC input with this distribution:

```text
12 V power supply
├── fuse / overcurrent protection
├── TMC2209 azimuth motor supply
├── TMC2209 elevation motor supply
└── regulated buck converter
    └── ESP32 board-supported supply input
```

Motor drivers remain on 12 V. Raw 12 V must never reach an ESP32 power or logic pin.
The buck converter must provide the voltage, current, ripple, and transient behavior
appropriate to the selected development board's documented input method. Do not
assume every board should be fed through a nominal 5 V pin.

The 12 V source, both drivers, buck converter, and ESP32 signal reference require a
common ground. Motor-current return paths should not share long, high-impedance runs
with logic or measurement returns. Place suitable bulk capacitance close to each
driver's motor-supply input and local decoupling close to logic electronics, following
the selected module and IC recommendations.

## Protection and commissioning

- Select fuse/overcurrent protection below the safe rating of the wiring, connectors,
  and weakest protected component.
- Add accessible power isolation for unexpected motion.
- Treat reverse-polarity protection as a recommended improvement before a public
  hardware revision.
- Verify connector polarity and continuity before energizing.
- Configure TMC2209 RMS motor current from the exact motor and module data; do not
  copy a nominal value from an unrelated build.
- Provide heatsinking and airflow based on driver temperature under the actual load.
- Separate motor/power wiring from STEP/DIR, switch inputs, receiver wiring, and RF
  feedlines where practical.

USB can energize an ESP32 while the external rail is present. The selected board and
buck topology must be reviewed for USB/external-power backfeeding before both are
connected. A jumper, ideal-diode/power-mux arrangement, or use of the board's protected
input may be required; there is no universal safe 5 V-pin rule.

The final power-supply current rating remains provisional until both motor ratings,
driver RMS-current settings, acceleration/load profile, ESP32 board, and future logic
loads are known. Include startup and stall margin without exceeding component ratings.
