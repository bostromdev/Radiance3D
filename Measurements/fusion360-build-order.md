# Fusion 360 build order — Revision 1

Work one subsystem at a time and pause for review at each numbered stage.

1. **Reconcile parameters.** Load the measured, calculated, design, and provisional
   parameter table. Confirm measured values are frozen.
2. **Purchased-component envelopes.** Model both motors, ESP32, two TMC2209s, two
   ZX-052s, and the selected AD8317 using measured envelopes and visible access faces.
3. **Component-envelope layout study.** Center the pan axis, solve platform size and
   electronics placement inside 220 × 220 mm, and present dimensions and swept volumes.
4. **D-shaft coupon.** Print a 5.00/4.60 mm D-profile split-clamp coupon and tune only
   `dHubProfileClearance`.
5. **Motor coupons.** Print the 21.97 mm pilot-register and 31.00 mm mounting-pattern
   coupons; use provisional M3-compatible holes.
6. **Stationary base.** Create the stable open base, centred pan-motor mount, side/rear
   strain-relieved power entry, distribution area, open wiring routes, and optional
   future bench-fastening points.
7. **Pan hub and platform.** Create a short-cantilever clamp hub and the smallest
   practical ribbed platform proven by the layout study.
8. **Open electronics retention.** Add removable rails/clips/straps/crossbars/zip-tie
   slots. Verify every display, connector, adjustment, terminal, and cooling envelope.
9. **Tilt motor and hub.** Mount the tilt motor compactly, align its shaft near the pan
   axis, and create a removable direct-drive clamp hub.
10. **AD8317/AUT carriage.** Add a vertical adjustable edge/strap holder, direct-SMA
    AUT clearance envelope, and lightweight nonconductive bending-load guide.
11. **Harness motion study.** Model neutral cable position, service loop, strain relief,
    one managed pan turn, tilt travel, and protection from hubs/platform/connectors.
12. **Engineering review.** Report base/platform dimensions, shaft cantilevers, moving
    mass estimate, CG offsets, tilt range, access conflicts, and direct-drive concerns.
13. **First structural print.** Print only after coupons and layout are approved; state
    PETG orientation and support strategy.
14. **Low-acceleration prototype.** Test balance, hub slip, shaft loading, cable twist,
    collision clearance, and base stability.
15. **Revision.** Change clearance/design parameters only; add new measured values
    without deleting historical readings.

Do not request bearings, separate shafts, couplers, exact PCB mounting holes, or final
wire lengths before conceptual CAD.
