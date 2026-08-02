# Fusion 360 BOM template

The enclosure has not yet been designed. All placements are conceptual only; use
[`../docs/hardware/reference-architecture.md`](../docs/hardware/reference-architecture.md)
as the official non-dimensioned baseline.

Use this as the starting BOM for the first prototype. Because the relevant hardware is already in the project, this can stay as a simple mechanical checklist first; fill in part numbers and suppliers only if you want a formal procurement list.

## Mechanical

| Item | Qty | Description | Part number | Notes |
|---|---:|---|---|---|
| Pan motor | 1 | NEMA 17 stepper, model 42HDB0014NC-24B | TBD | Existing hardware |
| Tilt motor | 1 | NEMA 17 stepper, model 42HDB0014NC-24B | TBD | Existing hardware |
| Pan bearing | 1 | Bearing for pan axis | TBD | Measure before CAD finalization |
| Tilt bearing | 1 | Bearing for tilt axis | TBD | Measure before CAD finalization |
| Pan coupler | 1 | Flexible or rigid shaft coupler | TBD | Measure before CAD finalization |
| Tilt coupler | 1 | Flexible or rigid shaft coupler | TBD | Measure before CAD finalization |
| Fasteners | TBD | M3 / M4 fasteners for mounts and base | TBD | Measure lengths before ordering |
| Heat-set inserts | TBD | Threaded inserts for plastic bosses | TBD | Measure OD and length |

## Electronics

| Item | Qty | Description | Part number | Notes |
|---|---:|---|---|---|
| ESP32 board | 1 | ELEGOO ESP32 devkit | TBD | Existing hardware |
| TMC2209 driver | 2 | Stepper driver module | TBD | Existing hardware |
| Buck converter A | 1 | ZX-052 V2.0, ESP32 5 V branch | TBD | Existing hardware; stationary base |
| Buck converter B | 1 | ZX-052 V2.0, AD8317 5 V branch | TBD | Existing hardware; stationary base |

## RF and antenna

| Item | Qty | Description | Part number | Notes |
|---|---:|---|---|---|
| Antenna under test | 1 | Threads directly onto AD8317 SMA | TBD | Measure before direct-SMA mount finalization |
| RF detector | 1 | AD8317 evaluation board, vertically mounted | TBD | Existing hardware; rotating platform |
| External VTX | 1 | Stationary external 5.8 GHz source | TBD | Off-scanner; not an enclosure component |
| RG316 jumper, antenna to detector | 0 | Not used | — | Direct SMA connection only |

## Miscellaneous

| Item | Qty | Description | Part number | Notes |
|---|---:|---|---|---|
| Limit switch | 2 | One per axis | TBD | Measure before assembly |
| Cable tie / strain relief | TBD | For routing and strain relief | TBD | Depends on final cable path |
