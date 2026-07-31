# Scan dataset format

The canonical version 1 format is JSON validated by
[`data/schemas/scan-v1.schema.json`](../../data/schemas/scan-v1.schema.json). The
[simulated example](../../data/examples/simulated/dipole-like-scan.json) is clearly
labeled and contains no physical measurement.

## Required record groups

- Identity: schema version, scan ID/name, scan timestamp, software, and firmware.
- Configuration: hardware, AUT, RF source, receiver, frequency, and transmit power.
- Experiment context: calibration reference, environmental notes, and warnings.
- Provenance: `measured`, `simulated`, `imported`, or `processed`, plus creator/method.
- Samples: timestamp, azimuth degrees, elevation degrees, value, unit, and flags.

`transmit_power` and `calibration_reference` are explicitly nullable so unknown is
not confused with zero or an empty object. Metadata objects allow additional fields
to accommodate device-specific information. Sample fields are strict so a typo
cannot silently create a second representation.

## Versioning

`schema_version` uses semantic versioning. Additive optional metadata is a minor
change; removing fields or changing meaning requires a new major schema file.
Readers must reject unsupported major versions. A CSV export may be added later, but
it must retain or accompany all dataset-level metadata.
