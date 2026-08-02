# Mechanical architecture

The planned mechanism has a base pan axis for azimuth and a supported tilt axis for
elevation. The AUT mount should provide a documented reference plane, repeatable zero,
strain relief, and interchangeable non-conductive fixtures where practical.

Design priorities are:

- low and characterized backlash;
- sufficient stiffness without placing unnecessary conductive mass near the AUT;
- controlled cable routing through the motion envelope;
- accessible hard stops, limits, and power isolation;
- separation of printable fixtures from bearings, shafts, and other parts better
  made from non-printed materials; and
- provisions for future alignment and calibration fixtures.

All choices are provisional until loads, required angular resolution, RF disturbance,
and repeatability are tested.

## Dimensions

Physical dimensions come from [`Measurements/`](../../Measurements/README.md), which is
the canonical source for the whole repository. Do not restate dimensions in mechanical
documents — link to the component file instead, so there is one place to correct.

- [NEMA 17 stepper motor](../../Measurements/nema17.md) — measured; short-body variant
- [TMC2209 V1.3 driver](../../Measurements/tmc2209-v1.3.md) — measured with heatsink installed
- [ESP32 devkit](../../Measurements/esp32-devkit.md) — measured

Bearings, shaft couplers, fasteners, heat-set inserts, the antenna mount and the limit
switches are **not measured**. They are the parts the mechanism depends on most, and
they block the design. See
[`missing-measurements.md`](../../Measurements/missing-measurements.md).

## CAD handoff

- [`fusion360-parameters.md`](../../Measurements/fusion360-parameters.md) — proposed
  user parameters, separated into measured, design choice, provisional and calculated
- [`fusion360-assistant-prompt.md`](../../Measurements/fusion360-assistant-prompt.md) —
  prompt for starting the pan-and-tilt prototype

A parameter marked **provisional** is a placeholder for a measurement that has not been
taken. Print a fit-test coupon before committing any feature that depends on one.
