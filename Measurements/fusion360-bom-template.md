# Revision-1 prototype BOM

## Owned hardware

| Item | Qty | Revision-1 role |
|---|---:|---|
| YEJMKJ 42HDB0014NC-24B NEMA-17 | 2 | direct-drive pan and tilt |
| ELEGOO ESP32 DevKit | 1 | stationary controller |
| BIGTREETECH TMC2209 V1.3 | 2 | stationary pan/tilt drivers |
| ZX-052 V2.0 | 2 | stationary independent 5 V branches |
| AD8317 EVAL BD / NWDZ V1.0 | 1 | vertical rotating detector, direct-SMA AUT |
| Interchangeable 5.8 GHz AUT | 1 at a time | direct threaded SMA connection |
| External stationary 5.8 GHz VTX | 1 | off-scanner RF source |
| Silicone wire, 18/22/26 AWG | as required | power, motor, control, analog harnesses |

## Prototype mechanical categories

| Item | Status |
|---|---|
| PETG base, platform, motor mounts, clamp hubs, carriage, trays, guides | design in Fusion |
| M3-compatible screws/nuts/inserts | provisional and editable; coupon before final bosses |
| Edge clips, straps, crossbars, and zip ties | reversible PCB retention |
| Side/rear 12 V strain relief and distribution hardware | select during base detail design |

## Explicitly not Revision 1

External bearings, separate shafts, shaft couplers, mandatory limit switches, slip
ring, LM2596 converters, third buck converter, internal battery, sealed enclosure, and
AUT-to-detector RG316 jumper.
