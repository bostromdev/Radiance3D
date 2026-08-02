# Data

- `schemas/` contains versioned machine-readable formats.
- `examples/` contains small, intentionally tracked examples.
- `raw/` is ignored because captures can be large and may contain sensitive setup
  details; preserve raw experiment data in an appropriate external archive.

Stored datasets must retain provenance and warnings. Raw files are immutable:
normalization, interpolation, visualization preparation, and export produce derived
artifacts that reference their source dataset IDs. Never relabel simulated or
processed output as a physical measurement.
