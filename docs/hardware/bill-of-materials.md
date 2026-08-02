# Bill of materials

There is no tested BOM. Version 1 categories include one ESP32 development board with
ESP-WROOM-32, two BIGTREETECH TMC2209 V1.3 modules, two YEJMKJ/LYLANMO NEMA 17
bipolar motors, two home switches, one emergency stop, one standalone 12 V battery
power source, buck conversion, overcurrent protection, bulk/local decoupling, driver
cooling, structure, bearings, fasteners, strain-relieved cabling, and independently
connected RF equipment. Exact part numbers and supply-current rating await motor,
current, thermal, and mechanical testing.

A publishable BOM must contain tested part numbers, revision, quantity, function,
acceptable substitutions, source date, and validation status. Price and availability
alone are not engineering validation.

## Measured parts

Three component families have been physically measured. The canonical dimensions live
in [`Measurements/`](../../Measurements/README.md); this page links to them rather
than duplicating the tables.

| Part | Identification | Qty | Dimensions |
|---|---|---:|---|
| NEMA 17 bipolar stepper | YEJMKJ / LYLANMO `42HDB0014NC-24B` | 2 | [`nema17.md`](../../Measurements/nema17.md) |
| TMC2209 V1.3 driver module | BIGTREETECH | 2 | [`tmc2209-v1.3.md`](../../Measurements/tmc2209-v1.3.md) |
| ESP32 devkit, ESP-WROOM-32, USB-C | ELEGOO | 1 | [`esp32-devkit.md`](../../Measurements/esp32-devkit.md) |

Not yet measured: home switches, emergency stop, LM2596 buck converters (SELOKY
LM2596 and LYLANMO LM2596S), bearings, shaft couplers, fasteners and heat-set
inserts, the antenna mount, and any RF detector. See
[`missing-measurements.md`](../../Measurements/missing-measurements.md).

Measurement is not qualification. Knowing a part's dimensions does not make it a
tested BOM entry.
