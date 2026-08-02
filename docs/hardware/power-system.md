# Power system

Version 1 uses a standalone 12 V automotive battery as the motor supply. It is not
connected to a running vehicle. The architecture is:

```text
12 V battery
├── 3–5 A inline fuse
├── master disconnect switch
├── power distribution
│   ├── TMC2209 VM
│   ├── TMC2209 VM
│   └── LM2596 buck converter
│       └── 5.0 V logic rail
│           └── ESP32
```

Motors receive 12 V directly from the battery rail. The ESP32 receives a regulated
5.0 V rail from the buck converter. The buck converter powers only logic electronics;
it is not intended to power the stepper motors. This architecture is used because the
motors need a robust 12 V supply and the ESP32 needs a clean, regulated logic supply
that is isolated from the motor-current return path.

The 12 V source, both drivers, the buck converter, and the ESP32 logic reference must
share a common ground. Motor-current return paths should not share high-impedance
runs with logic or measurement returns. Place a bulk capacitor near each TMC2209 VM
input and local decoupling near the ESP32 and logic electronics.

## Buck converter guidance

The confirmed modules are the SELOKY LM2596 and the LYLANMO LM2596S. Both are
adjustable DC-DC buck converters. Set the output with a multimeter before connecting
an ESP32. The output must be set to exactly 5.0 V, and the onboard display should not
be trusted as the only verification. Verify polarity and confirm that the buck output
and the ESP32 logic reference share a common ground.

## Capacitors and transients

Use one bulk capacitor near each TMC2209 VM input. An initial recommendation is
100–220 µF with a minimum 25 V rating. Add 0.1 µF ceramic decoupling where practical.
Stepper motors regenerate current when they are switched or decelerated. Supply
transients can therefore appear at the VM rail even when the command is static, so the
bulk capacitance and layout matter for noise and driver stability.

## Protection and commissioning

- Install an inline fuse sized for the wiring and the weakest protected component.
- Use a master disconnect switch that is visible and accessible.
- Use insulated terminals and avoid loose alligator clips.
- Verify connector polarity and continuity before energizing.
- Disconnect power before changing wiring.
- Never connect or disconnect motors while the drivers are powered.
- Configure TMC2209 RMS motor current from the exact motor and carrier data; do not
  copy a nominal value from an unrelated build.
- Separate motor/power wiring from STEP/DIR, switch inputs, and RF wiring where
  practical.

USB can energize an ESP32 while the external rail is present. Review the selected
board and buck topology for USB/external-power backfeeding before connecting both.
The Version 1 design assumes a protected 5.0 V input path and does not depend on the
buck converter to power the motors.
