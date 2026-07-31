# Motion system

Version 1 uses a base azimuth (pan) axis and a supported elevation (tilt) axis. The
AUT rotates; the measurement receiver/reference remains stationary. This arrangement
reduces receiver cable movement, cable strain, and position-dependent RF changes.
Cable routing still limits travel unless a future slip ring is tested and documented.

## Motors and drivers

Each axis uses one NEMA 17 bipolar stepper, provisionally assumed to be 1.8° full-step
only when configuration does not override it. Exact current rating, holding torque,
winding resistance, and required gearbox are unresolved.

TMC2209 is the Version 1 target driver. Prefer UART configuration so RMS motor current,
microstepping, and diagnostic state are reproducible. STEP, DIR, and enable are driven
by the ESP32. Current is never hardcoded before motor selection; it must be configured
from motor ratings, the module's sense-resistor implementation, load testing, and
thermal limits. Drivers need heatsinking/airflow appropriate to measured dissipation.

The firmware `MotionController` interface contains no TMC2209 type. A later physical
implementation may use that device or another stepper driver without changing the
serial or host scan API. Generic axis configuration reserves motor RMS current with
an unset simulator value rather than inventing a motor-specific default.

## Configuration-derived movement

Each axis records:

- motor full steps per revolution and microsteps;
- gear ratio and calculated steps per output revolution;
- direction inversion and home offset;
- minimum and maximum angle;
- maximum speed and acceleration; and
- home-switch polarity, debounce, direction, speed, back-off, and slow approach.

Gear or pulley dimensions are not assumed. The initial conceptual limits are azimuth
`0°` to `360°` and elevation `-90°` to `+90°`; real cable and structure clearances may
require smaller limits. The planner must reject every out-of-range point. It may
reverse alternate raster rows to reduce cable winding. Continuous/unlimited azimuth
rotation is not supported.

## Resolution and confidence

The project targets 0.1° commanded resolution and prioritizes repeatability over
speed. Motor/microstep quantization, mechanical resolution, repeatability, and absolute
accuracy are different quantities. Belt compliance, shaft play, backlash, frame
deflection, motor torque margin, and microstep nonlinearity must be measured. No
0.1° physical-accuracy claim follows from selecting a microstep value.

Version 1 position is open-loop step counting. A successful two-stage home establishes
trust. Reset, fault, emergency stop, driver disable, timeout, or suspected missed
steps invalidates it, and motion used for scanning requires re-homing. A home switch
does not observe position throughout a move. Future encoders can supply observed
position behind the same public API.

Normally-closed switches are recommended for broken-wire fault detection, while
polarity remains configurable. Emergency power isolation must not depend solely on
working firmware or a host connection.
