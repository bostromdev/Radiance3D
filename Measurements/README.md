# Measurements

This folder is the **canonical source for every physical dimension in Radiance3D**.

Revision 1 is an open direct-drive desktop prototype within a 220 × 220 mm maximum
base. Measurements support its Fusion layout; the
[reference architecture](../docs/hardware/reference-architecture.md) defines the fixed
subsystem architecture while Fusion determines placement.

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

| Component | Documentation | Source photos |
|---|---|---|
| NEMA 17 stepper motor (×2), model 42HDB0014NC-24B | [`nema17.md`](nema17.md) | IMG_5221, IMG_5223–5232, IMG_5247–5250 |
| BIGTREETECH TMC2209 V1.3 driver (×2) | [`tmc2209-v1.3.md`](tmc2209-v1.3.md) | IMG_5233–5240 |
| ELEGOO ESP32 devkit (×1) | [`esp32-devkit.md`](esp32-devkit.md) | IMG_5241–5246 |
| AD8317 EVAL BD (NWDZ V1.0) RF detector | [`ad8317.md`](ad8317.md) | IMG_5257–5262 |
| ZX-052 V2.0 buck converter (×2) | [`zx052-v2.0.md`](zx052-v2.0.md) | supplemental supplied photographs |

## CAD handoff

| Document | Purpose |
|---|---|
| [`fusion360-parameters.md`](fusion360-parameters.md) | Proposed Fusion 360 user parameters, separated into measured, design-choice, provisional and calculated |
| [`fusion360-assistant-prompt.md`](fusion360-assistant-prompt.md) | Complete prompt to paste into Fusion 360 Assistant to begin the pan-and-tilt prototype |
| [`fusion360-design-spec.md`](fusion360-design-spec.md) | A tighter, more precise Fusion 360 implementation spec for the first prototype |
| [`fusion360-build-order.md`](fusion360-build-order.md) | A step-by-step build order for modeling the pan/tilt prototype in Fusion 360 |
| [`fusion360-bom-template.md`](fusion360-bom-template.md) | A starter BOM template for the mechanical, electronics, RF, and fastener parts |

## Remaining provisional interfaces

Full detail, including how to take each one, is in
[`missing-measurements.md`](missing-measurements.md). Summary:

These do not block the component-envelope layout. Use reversible retention and named
provisional parameters where a printed feature actually depends on them:

- NEMA 17 mounting-hole diameter/screw specification and shaft-flat axial length/start
- TMC2209 heatsink length along the PCB long axis (one reading closes it)
- TMC2209 PCB thickness, header pitch, header row spacing
- ESP32 mounting holes and header pin length
- Fasteners and heat-set inserts — sizes and pilot holes
- Representative interchangeable AUT clearance envelopes
- ZX-052 total installed height and mounting-hole pattern; use open edge retention
- AD8317 detector — PCB thickness, mounting holes, shield-can height (outline is
  measured)

External bearings, separate shafts, couplers, and mandatory limit switches are not
Revision-1 parts. They do not block CAD and may be considered only as future upgrades.

## Source photographs

Every value cites the photograph it was read from, by filename.

**The photographs are not stored in this repository.** They were removed to keep the
working tree small. They live in `Desktop/fine tune measurements/Radiance3D-measurement-photos/`,
and remain in this repository's git history — `git log --all -- Measurements` will find
them if the working copy is ever lost.

The filename citations are kept regardless. A number that names its source is auditable
against the original photo; a number that doesn't is just an assertion.
