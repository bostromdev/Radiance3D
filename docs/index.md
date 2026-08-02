# Radiance3D documentation

Radiance3D is in architecture and prototyping. Documentation describes intended
interfaces unless a page explicitly marks behavior as implemented and tested.

## Start here

- [Architecture overview](architecture/overview.md)
- [Version 1 engineering baseline](architecture/version-1.md)
- [System boundaries](architecture/system-boundaries.md)
- [Data flow](architecture/data-flow.md)
- [Scan file format](software/file-formats.md)
- [Motion protocol](firmware/protocol.md)
- [Native ESP-IDF firmware architecture](firmware/esp-idf-architecture.md)
- [ESP-IDF migration notes](firmware/esp-idf-migration.md)
- [Firmware troubleshooting](firmware/troubleshooting.md)
- [Native ESP-IDF decision](architecture/adr-native-esp-idf.md)
- [TMC2209 commissioning](hardware/tmc2209-commissioning.md)
- [Physical measurements](../Measurements/README.md)
- [Fusion 360 parameters](../Measurements/fusion360-parameters.md)
- [Development setup](development/setup.md)
- [Roadmap](development/roadmap.md)
- [Glossary](glossary.md)

Hardware choices in this documentation are now recorded as a Version 1 engineering
baseline with explicit pending validation notes. Experiment documents define the
evidence required before measurement or accuracy claims can be made.

Physical dimensions are canonical in [`Measurements/`](../Measurements/README.md).
Documentation pages link there rather than restating dimensions, so a corrected
measurement propagates from one place. Measured values override generic online
dimensions, and dimensions that are still missing or unclear are tracked in
[`missing-measurements.md`](../Measurements/missing-measurements.md).
