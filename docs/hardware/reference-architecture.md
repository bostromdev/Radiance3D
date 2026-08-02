# Reference architecture — Revision 1 CAD design intent

## Status

**Conceptual architecture only. Final placement determined during CAD.** Revision 1
is an open desktop prototype within a 220 × 220 mm maximum base, not an enclosure or a
production machine. The subsystem boundaries and direct-drive mechanism are fixed;
Fusion solves the compact component layout and printed geometry.

## Design priorities

1. Measurement repeatability
2. Mechanical stability
3. RF consistency
4. Calibration repeatability
5. Serviceability
6. Manufacturability

## Architecture

```text
                 external stationary 5.8 GHz VTX
                              │ free-space RF
                              ▼
                 interchangeable Antenna Under Test
                              │ direct threaded SMA
                              ▼
                  vertically mounted AD8317 detector
                              │ lightweight tilt carriage
                              ▼
                     tilt NEMA-17 5 mm D-shaft
                 removable clamp hub; no coupler/bearing
                              │
                     DIRECT-DRIVE PAN PLATFORM
                              │ moving silicone harness
                 one managed turn from cable neutral
                              ▼
                      pan NEMA-17 5 mm D-shaft
                 removable clamp hub; no coupler/bearing
                              │
┌──────────────────────────────────────────────────────────────┐
│ DESIGN INTENT: STATIONARY BASE — open, maximum 220 × 220 mm │
│ centred pan motor; ESP32; pan + tilt TMC2209;               │
│ ZX-052 Buck A + Buck B; 12 V entry/distribution             │
└──────────────────────────────────────────────────────────────┘
                              │
                    off-board 12 V bench source
```

The **DESIGN INTENT: ROTATING PLATFORM** contains only the lightweight pan platform,
tilt motor, clamp hub and carriage, vertical AD8317, interchangeable AUT, cable guides,
and moving silicone harness.

## Direct drive

Each motor's internal bearings support the Revision-1 load. Revision 1 has no external bearings, separate shafts, or shaft couplers. A removable clamp-style hub engages the
measured 5.00 mm D-shaft without hard axial preload. CAD minimizes rotating mass,
cantilever distance, mechanical shock, acceleration, and centre-of-gravity offset.
The AD8317 PCB is retained by the carriage; it is not itself the structural hub.

Fusion must report pan and tilt cantilever distance, approximate rotating mass when
known, centre-of-gravity offset from both motor shafts, collision-free tilt travel, and
any direct-drive load concern. Direct drive is selected but not mechanically validated.
External bearing-supported axes are future upgrades only if prototype testing requires
them.

## RF and detector

The external stationary 5.8 GHz VTX remains off-scanner. The AUT directly threads onto
the AD8317 SMA; there is **no detector-to-antenna RG316 jumper**. The selected AD8317
EVAL BD / NWDZ V1.0 is vertical and moves with the AUT. Adjustable edge/strap retention
preserves both SMA connectors, electrical pads, and corner-hole areas. A lightweight,
preferably nonconductive guide may relieve antenna bending load without inserting coax.

## Stationary electronics and power

The centred pan motor, ESP32, two TMC2209s, both ZX-052 converters, power entry, and
distribution remain stationary. Boards are openly mounted with reversible trays, rails,
clips, straps, crossbars, or zip-tie slots; unknown hole coordinates do not block CAD.

External 12 V/GND feeds both TMC2209 VM/GND branches and two separate converters:
ZX-052 Buck A supplies regulated 5.0 V to the ESP32; ZX-052 Buck B supplies regulated
5.0 V to the AD8317. Their outputs are never paralleled. There is no third converter,
internal battery, battery compartment, or sealed electronics enclosure.

## Motion and harness

Pan targets one managed 360° turn from a neutral cable position, not unlimited
continuous rotation. The **Firmware-defined return-to-home strategy to prevent cumulative cable twist** follows the CAD motion study and prototype test. Tilt targets
the largest collision-free range; approximately −90° to +90° is desirable but not
guaranteed before CAD.

The moving silicone bundle contains four tilt-motor phases, AD8317 +5 V, GND, VOUT,
and analog return. CAD supplies a service loop, bend clearance, strain relief at both
ends, guides, and protection from the platform, hubs, and motor connectors. Final wire
lengths remain post-CAD measurements.

## Fusion layout obligation

Fusion first performs a component-envelope and swept-volume study, centers the pan
axis, finds the smallest practical pan platform, places stationary electronics outside
its sweep, proves all hardware stays inside 220 × 220 mm, and preserves every connector,
display, adjustment, terminal, cooling, and wiring-access direction. Exact placement is
a CAD result, not prescribed here.

## Future Expansion

External bearings, separate shafts, couplers, limit switches, a slip ring, guards,
alternate detectors, and other sensors are optional future work. None is required for
Revision 1.
