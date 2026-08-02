# CAD sources

No verified CAD model exists. Editable, revisioned source assemblies will be stored
here when dimensions and interfaces have been reviewed.

## Before modelling

Physical dimensions are canonical in [`Measurements/`](../../../Measurements/README.md).
Take every dimension from there — never from a vendor listing or a generic NEMA 17 or
StepStick drawing.

Start with:

1. [`fusion360-parameters.md`](../../../Measurements/fusion360-parameters.md) — build
   the Fusion user-parameter table from this before creating geometry.
2. [`fusion360-assistant-prompt.md`](../../../Measurements/fusion360-assistant-prompt.md) —
   the prompt for the first pan-and-tilt prototype.
3. [`missing-measurements.md`](../../../Measurements/missing-measurements.md) — what is
   still unknown, and therefore which features cannot yet be committed.

Parameters tagged **provisional** are placeholders standing in for measurements that
have not been taken. The bearing seats, the motor pilot register, the motor
mounting-hole pattern, the coupler bores and the heat-set insert bosses all currently
depend on provisional values. Print fit-test coupons for those before modelling around
them.

When a measurement is taken, update the component file in `Measurements/` first, then
`fusion360-parameters.md`, then the Fusion parameter. The measurement file stays the
single source of truth.
