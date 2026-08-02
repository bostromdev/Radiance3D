# Motion system

Version 1 uses a base azimuth axis and a supported elevation axis. The AUT rotates while
the measurement receiver/reference remains stationary. This arrangement reduces receiver
cable movement and position-dependent RF variation, but cable routing and structure
clearances still limit travel unless a future slip ring is tested and documented.

## Motors and drivers

Each axis uses one YEJMKJ/LYLANMO NEMA 17 bipolar 4-wire stepper. The selected motors
are 1.8° full-step, 200 full steps/revolution, rated at 1.0 A phase current, 3.5 Ω
phase resistance, and about 0.13 N·m holding torque. These values come from the
selected hardware and are not generic placeholders. Physical dimensions of the
actual motors are recorded in [`Measurements/nema17.md`](../../Measurements/nema17.md);
they are measured, not nominal, and the motors are the short-body variant.

Version 1 uses BIGTREETECH TMC2209 V1.3 drivers. They are UART controlled, use the
STEP/DIR interface, expose driver diagnostics, and allow the firmware to set current
through UART. The enable pin remains configurable, current is stored in RMS milliamps,
and current must never exceed the selected motor rating. The driver profile must be
validated against the actual carrier revision, R10 setting, sense resistor, UART
wiring, and pinout. See [TMC2209 commissioning](tmc2209-commissioning.md). Physical
dimensions of the actual driver modules, measured with the heatsink installed, are in
[`Measurements/tmc2209-v1.3.md`](../../Measurements/tmc2209-v1.3.md).

The firmware motion interface remains driver-neutral, so a future driver can preserve
its serial and host scan API. The current profile is intentionally conservative and
must be re-verified under load, temperature, and mechanical requirements.

## Configuration-derived movement

Each axis records:

- motor full steps per revolution and microsteps;
- gear ratio and calculated steps per output revolution;
- direction inversion and home offset;
- minimum and maximum angle;
- maximum speed and acceleration; and
- home-switch polarity, debounce, direction, speed, back-off, and slow approach.

The initial configuration is 16 microsteps and direct drive (1:1). That gives:

200 full steps/rev × 16 microsteps = 3200 microsteps/rev

and therefore a commanded microstep resolution of 0.1125° per microstep. This is a
commanded motion resolution and is not a guarantee of physical angular accuracy.
Backlash, frame rigidity, shaft coupling, microstep linearity, and motor repeatability
must be measured before any physical accuracy claim is made.

## Position confidence

Version 1 position is open-loop step counting. A successful homing sequence establishes
trust. Reset, faults, emergency stop, driver disable, timeout, or suspected missed
steps invalidate trust and require re-homing.

Home switches remain configurable as normally-open or normally-closed, with debounce,
fast approach, backoff, slow approach, home offset, position trust, and timeout
options still available in configuration. A home switch is a reference event, not a
continuous position sensor.
