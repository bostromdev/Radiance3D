# NEMA 17 stepper motor

Physical measurements of the actual motors used in Radiance3D Version 1. These values
override any generic NEMA 17 dimensions found online.

## Identification

- Component: NEMA 17 bipolar stepper motor, short-body ("pancake")
- Manufacturer: YEJMKJ / LYLANMO
- **Model: `42HDB0014NC-24B`** — read from the motor nameplate in `IMG_5249.HEIC` and
  `IMG_5250.HEIC`
- Quantity: 2 (azimuth / pan and elevation / tilt)
- Measurement source: photographs IMG_5221, IMG_5223–5232, IMG_5247–5250
- Measured with: digital caliper, IP54, mm mode
- Units: mm

### Nameplate data

Transcribed from the label, legible in `IMG_5249.HEIC` and `IMG_5250.HEIC`. These are
manufacturer figures, not measurements.

| Nameplate field | Value |
|---|---|
| Model | 42HDB0014NC-24B |
| Holding torque | 0.13 N·m |
| Phase resistance | 3.5 Ω |
| Phase inductance | 3.4 mH |
| Rated phase current | 1.0 A |
| Step angle | 1.8° |
| Frame size (nominal) | 42 × 42 × 21 mm |

### Phase wiring

The nameplate gives the coil pairing directly. Getting these pairs wrong on a TMC2209
makes the motor buzz, jitter or lock instead of turning.

| Wire colour | Phase |
|---|---|
| Black | A+ |
| Green | A− |
| Red | B+ |
| Blue | B− |

Coil A is **black + green**. Coil B is **red + blue**. Confirm continuity between each
pair with a multimeter before energising — it should read about 3.5 Ω within a pair and
open circuit between pairs.

## Measured dimensions

| Dimension | Value | Measurement source | Notes |
|---|---:|---|---|
| Motor body diagonal (corner to corner) | 54.30 mm | IMG_5223.HEIC | Across two opposite chamfered corners of the face plate |
| Motor body length (front face plate to rear face plate) | 20.84 mm | IMG_5249.HEIC | Excludes the front pilot boss. Earlier reading IMG_5231.HEIC gave 20.76 mm; the 0.08 mm spread is caliper repeatability |
| Motor body/rear-envelope repeat | 20.72 mm | supplemental photograph | Appears to use the same faces as 20.84 mm but the datum is not conclusive; retain both and use 20.84 mm for clearance |
| Overall length including pilot boss, excluding shaft | 22.85 mm | IMG_5248.HEIC | Boss face to rear face. **Supersedes IMG_5232**, whose decimals were unreadable |
| Overall length, shaft tip to rear face | 45.18 mm | IMG_5250.HEIC | The full envelope the motor occupies along its axis |
| Pilot boss diameter | 21.97 mm | IMG_5247.HEIC | The raised circular register that centres the motor in its mount |
| Output shaft length, from pilot boss face to shaft end | 22.39 mm | operator measurement — no photograph | Datum is the boss face, not the face plate. See below |
| Connector protrusion above end-cap face | 9.38 mm | IMG_5226/5227.HEIC | Earlier repeat measurements |
| Connector protrusion above end-cap face, latest | 9.53 mm | supplemental photograph | Use this larger measured envelope for CAD clearance |
| Motor connector housing envelope | 16.43 mm | IMG_5228.HEIC | Earlier measurement |
| Motor connector housing envelope, latest | 16.44 mm | supplemental photograph | Use latest value; 0.01 mm agreement is repeatability evidence |
| Motor front-face outside dimension, axis A | 43.46 mm | supplemental photograph | Use as the maximum body-clearance envelope, not a precision locator |
| Motor front-face outside dimension, other orientation | 42.43 mm | supplemental photograph | Do not average with 43.46 mm; pilot and holes provide precision location |
| Front-face mounting-hole centre spacing | 31.00 mm | supplemental photograph | Measured across one axis; use a nominally square 31.00 × 31.00 mm pattern unless later evidence differs |
| Output shaft major diameter | 5.00 mm | supplemental photograph | Measured on the owned motor |
| D-flat to opposite round surface | 4.60 mm | supplemental photograph | D-profile thickness, not flat depth |

## Derived from the measurements above

The axial measurements are over-determined, which lets the pilot boss height be
derived two independent ways. They agree to 0.06 mm — normal caliper repeatability —
so the geometry is self-consistent.

| Derived value | Result | Expression |
|---|---:|---|
| Pilot boss height (route A) | 2.01 mm | `22.85 − 20.84` (boss+body, minus body) |
| Pilot boss height (route B) | 1.95 mm | `45.18 − 22.39 − 20.84` (total, minus shaft, minus body) |
| **Pilot boss height (adopted)** | **2.00 mm ±0.06** | both routes bracket it; equals the NEMA 17 nominal |
| Shaft length from the **face plate** | 24.39 mm | `22.39 + 2.00` (shaft above boss, plus boss height) |
| D-flat radial depth | 0.20 mm | `(5.00 − 4.60) / 2`; derived, not separately measured |

Both routes bracket 2.00 mm, which is also the NEMA 17 nominal, so 2.00 mm is adopted
with a ±0.06 mm measurement tolerance. That is well inside what a PETG print can hold —
FFF dimensional variation is normally larger — so it is adequate for prototype CAD.

**This value is frozen.** If a printed register comes out tight, change
`dPrintClearance`, never `mMotorPilotHeight`. See the fit rule in
[`fusion360-parameters.md`](fusion360-parameters.md).

### Shaft length datum

The 22.39 mm reading is taken from the top of the pilot boss to the end of the shaft.
It is **not** what a vendor listing means by "shaft length", which is measured from the
motor face plate. That figure is now derivable at 24.39 mm, but it inherits the 0.06 mm
uncertainty in the boss height, so prefer the 22.39 mm boss datum where a feature can
be dimensioned from the boss.

This value is the only one in this folder with no source photograph — it was reported
directly rather than read from a caliper picture. A photo would close that gap.

## CAD notes

- **Important protrusions:** the rear connector is the dominant obstruction. Use the
  larger 9.53 mm protrusion and 16.44 mm housing envelope. Any pan or tilt
  motor pocket must clear both, plus the mating plug and wire bend radius. With bend
  12.00 mm bend clearance gives 21.53 mm behind any motor, which helps decide whether a
  motor can face shaft-up.
- **Axial envelope:** 45.18 mm shaft tip to rear face, plus the connector allowance
  behind it. Budget from this figure, not from the body length.
- **Pilot register:** 21.97 mm diameter, 2.00 mm high. The counterbore in the printed
  mount is sized from these. This is the feature that centres the motor, so it is
  worth a fit-test coupon.
- **Body length:** at 20.84 mm the motor is a short-body NEMA 17, matching the
  nameplate's 42 × 42 × 21 mm. Do not design around a 34 mm or 48 mm body.
- **Mounting-hole layout:** 31.00 mm centre spacing is measured on one axis. Treat the
  pattern as nominally square for Revision 1; hole diameter remains provisional M3-compatible.
- **Body envelope:** use 43.46 mm plus clearance for a loose body pocket. Do not make
  the outer body a locating fit; the 21.97 mm pilot and hole pattern locate the motor.
- **Shaft:** the 5.00 mm major diameter and 4.60 mm D thickness are verified. Revision 1
  uses a removable clamp-style direct-drive hub, not a coupler. The exact flat axial
  length/start remains adjustable in the hub model.
- **Required print clearance:** use a fit-test coupon before committing a bore or
  pocket. See [`fusion360-parameters.md`](fusion360-parameters.md).
- **Unclear or missing dimensions:** see [`missing-measurements.md`](missing-measurements.md).

## Values read but not yet usable

These display values are legible, but the caliper jaw position in the photo does not
establish which feature was measured, or the decimals are not legible. They are
recorded here for traceability. **Do not use them in CAD.**

| Reading | Measurement source | Problem |
|---:|---|---|
| illegible (`4?.??`) | IMG_5224.HEIC | LCD blown out by glare; neither digit pair readable |
| 4.9? mm | IMG_5225.HEIC | Jaws are on the output shaft, so this is shaft diameter, but the second decimal is not legible |
| 1.65 mm | IMG_5229.HEIC | Value legible; jaws are near the rear end cap / connector but the feature is not identifiable from the frame |
| 4.00 mm | IMG_5230.HEIC | Value legible; jaws are at the rear corner near the connector, feature not identifiable |
| 22.11 or 22.71 mm | IMG_5232.HEIC | **Superseded.** Intended as boss-face-to-rear-face; IMG_5248 measures that cleanly at 22.85 mm. Retained only for traceability |
