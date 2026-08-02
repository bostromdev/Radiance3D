# Reference architecture — CAD design intent

## Status and terminology

**Conceptual architecture only. Final placement determined during CAD.** The enclosure
has not yet been designed, so this document identifies what connects and the intended
subsystem boundaries—not dimensions, mounting coordinates, cable lengths, or final
board orientations.

### Verified Hardware

Verified Hardware is physically owned, photographed or measured, or electrically
confirmed. The authoritative list, photographs, electrical facts, and harness standards
remain in [owned hardware](owned-hardware.md), [power tree](power-tree.md), and
[electrical/RF cabling standard](wire-standard.md).

### Design Intent

Design Intent is the planned enclosure layout, cable routing, conceptual subsystem
placement, and board orientation that Autodesk Fusion will resolve. It is not a claim
that any mechanical arrangement already exists.

## PRIMARY DESIGN OBJECTIVE

Radiance3D is a precision RF measurement instrument. Every mechanical decision should
prioritize, in order:

1. Measurement repeatability
2. Mechanical rigidity
3. RF consistency
4. Calibration repeatability
5. Serviceability
6. Manufacturability

Cosmetic appearance is secondary.

## Conceptual subsystem boundaries

```text
                         external stationary 5.8 GHz VTX
                                  │ RF transmission through air
                                  ▼
                    AUT threads directly onto AD8317 SMA
                                  │
                 ┌───────────────────────────────────┐
                 │ DESIGN INTENT: ROTATING PLATFORM   │
                 │  • tilt motor                      │
                 │  • AD8317 detector                 │
                 │  • antenna mount / AUT             │
                 └───────────────────────────────────┘
                                  │
       controlled rotation intent │ moving silicone harness only
      (one 360° turn, not unlimited│ • tilt motor wiring
           continuous rotation)   │ • AD8317 5 V/GND and VOUT/AGND
                                  │
                 ┌───────────────────────────────────┐
                 │ DESIGN INTENT: STATIONARY BASE     │
                 │  • fixed pan motor                 │
                 │  • ESP32 DevKit                    │
                 │  • TMC2209 pan + tilt drivers      │
                 │  • ZX-052 Buck A and Buck B        │
                 │  • internal 12 V distribution      │
                 │  • +12V IN / GND IN entry          │
                 └───────────────────────────────────┘
                                  │
                        off-board 12 V bench source
```

### Electrical architecture — Verified Hardware

The external two-conductor 12 V input feeds internal distribution, two TMC2209 VM/GND
branches, and the two independent ZX-052 5 V branches. Grounds share one defined
reference and the two 5 V outputs are never connected together. These are electrical
rules, not enclosure-placement instructions.

### RF architecture — Verified Hardware and Design Intent

The external stationary 5.8 GHz VTX is not mounted on the scanner. **Design Intent:**
the AUT directly threads onto the AD8317 SMA, with no detector-to-antenna RG316 jumper.
The AD8317 `VOUT` is `SIG-003` silicone wire back to the stationary ESP32 ADC. Exact
detector orientation, AUT clearance, and VTX test geometry remain CAD/bench work.

### Mechanical intent — Design Intent

The stationary-base and rotating-platform contents above are intended subsystem
boundaries. The pan motor is intended to remain in the stationary base; the moving
harness is intended to carry only tilt-motor and AD8317 wiring. One controlled 360°
rotation is the architecture target, not unlimited continuous rotation.

Final CAD determines mounting locations, enclosure form, cable-guide geometry, service
loop size, strain-relief method, bend radii, bearings, clearances, and all cable lengths.
The return-to-home behavior required to prevent cumulative cable twist is described as
the **Firmware-defined return-to-home strategy to prevent cumulative cable twist**. Its
implementation and safe limits remain deferred until physical testing.

## Future CAD work

Fusion design will translate this conceptual architecture into measured interfaces and
then document dimensions, hardware retention, cable routing, and service access. No
unknown geometry is validated by this repository before that work exists.

## Future Expansion

### Not Required for Revision 1

The following are informational possibilities only. They do not change Revision 1
hardware, firmware, GPIO, electrical architecture, or BOM: slip ring support,
alternate detector modules, alternate ESP32 variants, camera mounting, different motors,
protective covers, larger antennas, and additional sensors.
