# Scan dataset format

The canonical version 1 format is JSON validated by
[`data/schemas/scan-v1.schema.json`](../../data/schemas/scan-v1.schema.json). The
[simulated example](../../data/examples/simulated/dipole-like-scan.json) is clearly
labeled and contains no physical measurement.

Schema `1.0.0` remains readable for migration. New datasets use `1.1.0`, which adds
the complete Version 1 synchronization and provenance fields.

## Required record groups

- Identity: schema version, scan ID/name, scan timestamp, software, firmware,
  protocol version, and hardware revision.
- Configuration: hardware, AUT, RF source, receiver, frequency, and transmit power.
- Scan configuration: commanded azimuth/elevation step sizes and native measurement
  units.
- Experiment context: calibration status/reference, operator notes, environmental
  notes, and warnings.
- Provenance: `measured`, `simulated`, `imported`, or `processed`, plus creator/method.
- Samples: contiguous sequence number, timestamp, azimuth/elevation degrees, native
  value/unit, measurement source, commanded/observed position kind, validity, warnings,
  and quality flags.

`transmit_power` and `calibration_reference` are explicitly nullable so unknown is
not confused with zero or an empty object. Metadata objects allow additional fields
to accommodate device-specific information. Sample fields are strict so a typo
cannot silently create a second representation.

`step_size_deg` is the commanded raster increment, not demonstrated physical
resolution or accuracy. `measurement_units` must exactly match units found in samples.
Calibration status is explicit and does not imply true gain.

## Versioning

`schema_version` uses semantic versioning. Schema 1.1 makes new fields mandatory only
for 1.1 records, while readers retain 1.0 support. Removing fields or changing meaning
requires a new major schema file. Readers reject unsupported versions. A CSV export
may be added later, but it must retain or accompany all dataset-level metadata.
