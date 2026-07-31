# Motion system

The conceptual arrangement uses a base azimuth axis and a supported elevation axis.
Stepper drivers such as TMC2209 and NEMA 17-class motors are candidates only.
Selection depends on inertia, gear ratio, holding torque, current, thermal behavior,
backlash, resolution, and RF emissions.

Each axis needs a repeatable zero strategy, conservative soft limits, and preferably
independent limit inputs. Emergency power isolation must not depend solely on working
firmware or a host connection.
