# Measurements

This folder is the **canonical source for every physical dimension in Radiance3D**.

Every number here was measured with a digital caliper on the actual parts in hand and
is traceable to a photograph in this folder. Hardware, mechanical, assembly, BOM and
CAD documents elsewhere in the repository must link here rather than repeat these
numbers, so there is exactly one place to correct when a part changes.

> [!IMPORTANT]
> **Physical measurements override generic online dimensions.** Vendor listings,
> datasheets and "standard" NEMA 17 or StepStick figures describe a family of parts,
> not the specific parts on this bench. Where a measured value and a nominal value
> disagree, the measured value wins.

> [!WARNING]
> **Values marked unclear must not be used for final CAD.** Several caliper photos are
> affected by glare or resolution limits. Those readings are recorded for traceability
> only and are listed in [`missing-measurements.md`](missing-measurements.md). Print a
> fit-test coupon before committing any dimension that depends on them.

All dimensions are in millimetres.

## Measured values are frozen

Every critical dimension exists twice: the **measured value**, which never changes
unless the part is re-measured or replaced, and the **CAD value**, which is an
expression built from it.

```text
mMotorPilotHeight       = 2.00 mm                            <- measured, frozen
cMotorPilotPocketDepth  = mMotorPilotHeight + dPrintClearance  <- CAD, tune here
```

When a printed part fits badly, the fix is a clearance parameter — never the measured
number. Editing a measured value to chase a fit destroys the record of what the
hardware actually is, and the error then propagates into every feature referencing it.
See [`fusion360-parameters.md`](fusion360-parameters.md).

## Measured components

| Component | Documentation | Source pictures |
|---|---|---|
| NEMA 17 stepper motor (×2), model 42HDB0014NC-24B | [`nema17.md`](nema17.md) | `Nema-17/` — IMG_5221.jpg, IMG_5223–IMG_5232.HEIC, IMG_5247–IMG_5250.HEIC |
| BIGTREETECH TMC2209 V1.3 driver (×2) | [`tmc2209-v1.3.md`](tmc2209-v1.3.md) | `TMC2209 v1.3/` — IMG_5233–IMG_5240.HEIC |
| ELEGOO ESP32 devkit (×1) | [`esp32-devkit.md`](esp32-devkit.md) | `ESP32-ELEGOO/` — IMG_5241–IMG_5246.HEIC |
| AD8317 EVAL BD (NWDZ V1.0) RF detector | [`ad8317.md`](ad8317.md) | `AD8317/` — IMG_5257–IMG_5262.HEIC |

## CAD handoff

| Document | Purpose |
|---|---|
| [`fusion360-parameters.md`](fusion360-parameters.md) | Proposed Fusion 360 user parameters, separated into measured, design-choice, provisional and calculated |
| [`fusion360-assistant-prompt.md`](fusion360-assistant-prompt.md) | Complete prompt to paste into Fusion 360 Assistant to begin the pan-and-tilt prototype |

## Measurements still needed

Full detail, including how to take each one, is in
[`missing-measurements.md`](missing-measurements.md). Summary:

**Unclear — re-shoot:**

- NEMA 17 face width across flats (decimals unreadable)
- NEMA 17 shaft diameter (second decimal unreadable)
- NEMA 17 — one completely unreadable display, and two legible values whose measured
  feature cannot be identified

**Not measured at all:**

- NEMA 17 mounting-hole spacing and diameter — the largest remaining unknown on the
  motor; every other motor-mount dimension is now measured
- NEMA 17 shaft flat length and flat depth
- TMC2209 heatsink length along the PCB long axis (one reading closes it)
- TMC2209 PCB thickness, header pitch, header row spacing
- ESP32 mounting holes and header pin length
- Bearings — inner diameter, outer diameter, width
- Shaft couplers — bores, outer diameter, length
- Fasteners and heat-set inserts — sizes and pilot holes
- Antenna mount — connector, antenna body, coax diameter and bend radius
- Limit switches — body, mounting holes, lever, actuation travel
- LM2596 buck converter — outline, mounting holes, total height
- AD8317 detector — PCB thickness, mounting holes, shield-can height (outline is
  measured)

Bearings, couplers, fasteners, the antenna mount and the limit switches have no
measurements at all yet, so no component file has been created for them. Files will be
added when the parts are measured — empty placeholder files are deliberately not
created.

## Photograph handling

Source pictures are never modified, renamed or deleted. The original filename is
recorded as the measurement source for every value, so any number in this repository
can be traced back to the photograph it came from.

Most pictures are Apple HEIC. If a viewer cannot open them, convert a copy — do not
convert in place.
