# Missing and unclear measurements

Everything on this page blocks CAD work. Nothing here may be used as a dimension in
Fusion 360 until it is re-measured and recorded in the relevant component file.

Two categories:

1. **Unclear** — a photo exists, but the value or the measured feature cannot be read
   from it with confidence.
2. **Not measured** — no photo exists for the part or feature at all.

---

## 1. Unclear — re-shoot or re-measure

### NEMA 17 — motor face width

- **Component:** NEMA 17 stepper
- **Missing dimension:** face width across flats (the 42 mm nominal dimension), to two decimals
- **Why it matters:** every motor pocket, face plate and bolt-circle in the pan and
  tilt mounts is dimensioned from this. A 0.3 mm error here shows up as a rocking or
  binding motor.
- **How to measure:** caliper across two opposite flats of the face plate, not the
  chamfered corners. Take it twice, rotated 90°, and record both.
- **Related picture:** `Nema-17/IMG_5221.jpg` — reads `42` but the photo is too low
  resolution to resolve the decimals.

### NEMA 17 — unreadable display

- **Component:** NEMA 17 stepper
- **Missing dimension:** unknown — the display is unreadable
- **Why it matters:** an entire measurement is lost.
- **How to measure:** repeat the shot with the LCD out of direct light. Tilt the
  caliper so the light source is behind the camera, not reflecting off the screen.
- **Related picture:** `Nema-17/IMG_5224.HEIC` — LCD blown out by glare; reads `4?.??`.

### NEMA 17 — shaft diameter

- **Component:** NEMA 17 stepper
- **Missing dimension:** output shaft diameter, second decimal
- **Why it matters:** the shaft coupler bore and any printed hub are sized from this.
  Getting it wrong by 0.1 mm is the difference between a slip fit and a press fit.
- **How to measure:** caliper across the round part of the shaft, clear of any flat.
  Take three readings at 60° apart and record all three — a stepper shaft is not
  perfectly round.
- **Related picture:** `Nema-17/IMG_5225.HEIC` — reads `4.9?`.

### NEMA 17 — two unidentified features

- **Component:** NEMA 17 stepper
- **Missing dimension:** the values `1.65 mm` and `4.00 mm` are legible but the photos
  do not show which feature the jaws were on
- **Why it matters:** they are probably the pilot boss height and an end-cap or
  connector dimension, both of which the motor mount depends on. Guessing which is
  which would put a wrong number into CAD.
- **How to measure:** re-take each shot from further back so both the jaws and the
  display are in the same frame, then note in the filename or a text file what was
  measured.
- **Related pictures:** `Nema-17/IMG_5229.HEIC`, `Nema-17/IMG_5230.HEIC`.

### NEMA 17 — overall length including pilot boss

- **Component:** NEMA 17 stepper
- **Missing dimension:** overall length front boss face to rear face — reads either
  `22.11 mm` or `22.71 mm`
- **Why it matters:** subtracting this from the 20.76 mm body length is how the pilot
  boss height is derived. The two candidate readings give a 1.35 mm or a 1.95 mm boss —
  a 0.6 mm difference in how deep the register counterbore must be.
- **How to measure:** re-shoot with the LCD square to the camera and well lit from
  behind the camera. Better still, measure the boss height directly with a depth gauge.
- **Related picture:** `Nema-17/IMG_5232.HEIC`.

---

## 2. Not measured — no photo exists

### NEMA 17 — mounting-hole pattern

- **Missing dimension:** mounting-hole spacing (centre to centre) and hole diameter
- **Why it matters:** this is the single most important dimension for both motor
  mounts. The NEMA 17 standard is 31.0 mm square with M3 clearance holes, but that is
  a nominal standard, not a measurement of these motors.
- **How to measure:** caliper between the outside edges of two adjacent holes, then
  subtract one hole diameter, to get centre-to-centre. Measure the hole diameter with
  the caliper's internal jaws.

### NEMA 17 — pilot boss

- **Missing dimension:** pilot boss diameter and height
- **Why it matters:** the boss is what centres the motor in its mount. The
  counterbore in the printed part is sized directly from it.
- **How to measure:** caliper across the raised circular boss for diameter; depth
  gauge or caliper depth rod against the face plate for height.

### NEMA 17 — shaft length and flat

- **Missing dimension:** shaft length from the face plate, flat length, flat depth
- **Why it matters:** determines coupler engagement, whether a grub screw lands on the
  flat, and how much shaft the tilt cradle can use.
- **How to measure:** caliper depth rod from the front face plate to the shaft end for
  length; caliper across the shaft on the flat, compared to the round diameter, for
  flat depth.

### TMC2209 V1.3 — one reading closes the heatsink footprint

- **Missing dimension:** heatsink **back** edge to PCB **front** edge, along the PCB
  long axis — the counterpart to the 13.73 mm reading in IMG_5237
- **Why it matters:** the width pair (12.21 and 11.22) already gives a heatsink width
  of 8.29 mm and a 0.495 mm offset from the PCB centre. Along the long axis only one
  reading exists, so the heatsink's front edge is known (6.41 mm from the PCB front
  edge) but its length is not. Without it the heatsink footprint cannot be modelled,
  only its height.
- **How to measure:** exactly as IMG_5237 was taken, but with the jaws referenced from
  the opposite pair of edges. The two readings should sum to more than the 20.14 mm
  PCB length; the excess is the heatsink length.
- **Related picture:** `TMC2209 v1.3/IMG_5237.HEIC` gives the first half of the pair.

### TMC2209 V1.3 — remaining features

- **Missing dimensions:** PCB thickness, header pin pitch, header row spacing, header
  pin length below the board
- **Why it matters:** needed to design a driver bay, a retention clip, or a socket
  strip position rather than just an outer envelope.
- **How to measure:** caliper on a clear PCB edge for thickness; caliper across the
  outer pins of one row and divide by the pin count minus one for pitch; caliper
  between the two rows for row spacing.

### ESP32 devkit — mounting holes and pin length

- **Missing dimensions:** mounting-hole diameter and spacing, header pin length below
  the board, USB-C connector protrusion past the board edge
- **Why it matters:** the electronics tray needs standoff positions, a clearance
  depth below the board, and a correctly placed USB opening.
- **How to measure:** caliper internal jaws for hole diameter; caliper between two
  hole outer edges minus one diameter for spacing; caliper depth rod for pin length.

### Bearings — nothing measured

- **Missing dimensions:** inner diameter, outer diameter, width, and quantity, for
  both the pan bearing and the tilt bearing
- **Why it matters:** the bearing seats are the most fit-critical features in the
  entire assembly. Without these there is no pan platform and no tilt support.
- **How to measure:** caliper across the outer race for OD, internal jaws in the bore
  for ID, and across the race faces for width. Record the bearing's printed
  designation (e.g. `608ZZ`, `6800`) as well.

### Shaft couplers — nothing measured

- **Missing dimensions:** bore diameters (both ends), outer diameter, overall length,
  grub-screw size and position
- **Why it matters:** sets the axial distance between the motor and the driven shaft,
  which sets the height of the whole tilt assembly.
- **How to measure:** caliper OD and length; internal jaws or gauge pins for each bore.

### Fasteners — nothing measured

- **Missing dimensions:** screw sizes and lengths for the motor mounts, the base, and
  the antenna mount; heat-set insert outer diameter, length, and required pilot hole
- **Why it matters:** heat-set insert bosses cannot be modelled without the insert's
  OD and length. Screw lengths determine boss depth.
- **How to measure:** caliper across the thread crest for nominal size, along the
  shank for length. For inserts, caliper the knurled OD at its widest point and the
  overall length, then check the manufacturer's recommended pilot diameter.

### Antenna mount — nothing measured

- **Missing dimensions:** connector type and thread size (SMA / RP-SMA / N), antenna
  body diameter and length, required cradle clamp dimensions, coax outer diameter and
  minimum bend radius
- **Why it matters:** the antenna cradle and the adjustable mount cannot be designed
  at all without these. The coax bend radius also constrains the whole cable route.
- **How to measure:** caliper on the antenna body and the connector; for bend radius,
  use the coax manufacturer's specification rather than bending the cable to find out.

### Limit switches — nothing measured

- **Missing dimensions:** switch body length, width, height, mounting-hole spacing and
  diameter, lever length, actuation travel
- **Why it matters:** homing repeatability depends on rigid, repeatable switch
  mounting. The switch pockets and their adjustment slots come from these numbers.
- **How to measure:** caliper on the switch body and between the mounting holes.

### LM2596 buck converter — nothing measured

- **Missing dimensions:** PCB length, width, thickness, mounting-hole spacing,
  total height including the inductor, terminal-block height
- **Why it matters:** the buck converter shares the electronics tray with the ESP32
  and the two drivers. Two different modules are in use (SELOKY LM2596 and LYLANMO
  LM2596S) and they may not share an outline — measure both.
- **How to measure:** caliper on the PCB outline and across the tallest component.

### AD8317 / AD8318 RF detector — nothing measured

- **Missing dimensions:** PCB outline, thickness, mounting holes, RF connector type
  and position, total height
- **Why it matters:** needed only once an RF detector is actually selected. No
  detector is committed to in Version 1 — see
  [`../docs/hardware/rf-measurement.md`](../docs/hardware/rf-measurement.md).
- **How to measure:** as for the other PCBs, once a board is in hand.
