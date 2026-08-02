# Radiance3D owned hardware baseline

This repository uses only the owned-hardware profile in
[`firmware/config/radiance3d-owned-hardware.json`](firmware/config/radiance3d-owned-hardware.json).
The hardware photographs in [`Part Numbers:Views/`](<Part Numbers:Views/>) are the
visual authority; this file is an index, not a substitute for inspection.

**Verified Hardware** is owned, photographed, measured, or electrically confirmed.
**Design Intent** fixes the Revision-1 subsystem architecture while leaving the exact
layout to Fusion. Revision 1 is an open desktop prototype: a centred stationary pan
motor directly drives a lightweight platform, and the rotating tilt motor directly
drives the AD8317/AUT carriage. It has no external bearings, separate shafts, couplers,
slip ring, sealed enclosure, or battery compartment. The
[reference architecture](docs/hardware/reference-architecture.md) keeps verified facts
and CAD choices separate.

Read the hardware documentation in this order:

1. [Owned hardware and BOM](docs/hardware/owned-hardware.md)
2. [Power tree](docs/hardware/power-tree.md)
3. [GPIO map](docs/hardware/gpio-map.md)
4. [Wire and harness standard](docs/hardware/wire-standard.md)
5. [Assembly, routing, and commissioning](docs/hardware/assembly-order.md)
6. [Conceptual Fusion reference architecture](docs/hardware/reference-architecture.md)

The firmware uses ESP-IDF. It has no calibrated RF-power conversion and has not yet
been validated on the physical mechanism. Firmware soft limits are deliberately
conservative placeholders until the final cable route is measured.
