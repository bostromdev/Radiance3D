# Hardware

Hardware content is architectural and provisional. There are no verified schematics,
PCBs, wiring diagrams, CAD models, or released printable parts.

- `electronics/` describes where reviewed electrical design files will live.
- `mechanical/` records mechanical requirements and future design locations.
- `wiring/` defines documentation expectations before energizing a prototype.
- `bom/` defines evidence required before publishing a tested bill of materials.

## Physical dimensions

[`Measurements/`](../Measurements/README.md) is the canonical source for every physical
dimension in this project. Hardware documents link there instead of repeating numbers.

Measured: [NEMA 17 motor](../Measurements/nema17.md),
[TMC2209 V1.3 driver](../Measurements/tmc2209-v1.3.md),
[ESP32 devkit](../Measurements/esp32-devkit.md).

Not measured: bearings, couplers, fasteners, antenna mount, limit switches, LM2596
buck converters, RF detector — see
[`missing-measurements.md`](../Measurements/missing-measurements.md).

Future verified hardware files may receive a separate licence. See
[LICENSE](../LICENSE) for the terms that currently apply.
