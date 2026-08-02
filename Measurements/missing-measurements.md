# Remaining provisional measurements

Revision 1 can proceed to a component-envelope layout without the values below. Use
reasonable, reversible prototype geometry and a named `p...` parameter only when a
feature depends on an unverified value. Unknown PCB mounting holes are handled with
edge clips, shallow trays, straps, rails, removable crossbars, or zip-tie slots.

## Direct-drive motor interfaces

- **Motor hole diameter / screw:** spacing is verified at 31.00 mm nominal square;
  use an editable M3-compatible clearance parameter for the prototype mount.
- **Shaft flat axial length/start:** the D-profile is verified at 5.00 mm major diameter
  and 4.60 mm flat-to-opposite thickness. Keep clamp-hub engagement adjustable and
  verify with a D-shaft fit coupon. Do not use the ambiguous approximately 6 mm image
  as a hard axial dimension.
- **Fasteners/inserts:** use editable M3-compatible provisional parameters and a coupon
  before a serviceable joint is committed.

These values affect final hub or fastener fit, not the layout study.

## Open stationary electronics

- **ZX-052:** total installed height and mounting-hole pattern remain unknown. Model
  the verified 66.07 × 36.48 mm PCB with a conservative open height envelope and
  reversible edge retention. Preserve terminals, display, adjuster, pushbutton, tall
  components, ventilation, and 18/22 AWG wire bends.
- **ESP32:** mounting-hole coordinates, header-pin depth, USB-C projection, and cable
  overmould are unmeasured. Use measured PCB/USB envelopes with rails and an end clip.
- **TMC2209:** heatsink long-axis extent, PCB/header geometry, and final retention are
  incomplete. Use the measured PCB and 22.24 mm installed-height envelope with open
  cooling/access.

None of these blocks the open-base layout.

## AD8317 and AUT

- **AD8317:** PCB thickness, total component height, exact hole coordinates/diameters,
  and SMA centre locations remain unverified. The approximately 40.37 mm diagonal
  feature readings are not a hole pattern. Use adjustable edge/strap retention and
  preserve the four corner-hole regions.
- **AUT:** Revision 1 supports interchangeable 5.8 GHz antennas. Use a parameterized
  clearance envelope, adjustable detector position, and minimally intrusive support;
  validate later with representative antennas.

The selected AD8317 outline, direct SMA arrangement, and vertical orientation are
sufficient for conceptual and prototype geometry.

## Harness and prototype results

Final moving-bundle diameter, bend radius, service-loop length, pan/tilt soft limits,
rotating mass, shaft cantilever, centre-of-gravity offsets, and collision-free tilt
range are outputs of CAD and low-acceleration prototype testing. They are not reasons
to stop the initial model.

## Historical items excluded from Revision 1

External bearings, separate shafts, shaft couplers, LM2596 converters, mandatory limit
switches, a slip ring, an internal battery, a sealed enclosure, and AUT-to-detector
coax are not Revision-1 requirements. Do not create provisional geometry for them.
