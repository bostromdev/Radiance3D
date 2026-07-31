# Changelog

All notable project changes will be documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project intends
to use semantic versioning once releases begin.

## [Unreleased]

### Added

- Initial repository architecture, documentation, schema, software package,
  firmware simulator foundation, and quality checks.
- Complete Version 1 engineering baseline for ESP32, dual NEMA 17 axes, driver-neutral
  TMC2209 target architecture, power, wiring, RF geometry, and scope boundaries.
- Host `MotionController` and `MeasurementAdapter` protocols, bounds-safe serpentine
  raster planning, and move-settle-measure coordination that preserves raw/rejected
  readings.
- Firmware motion-controller abstraction with configuration-derived command
  quantization, configured limits, homing, position confidence, enable, stop, and
  emergency-stop simulator behavior.
- Native firmware behavior tests and software planner/coordinator tests.

### Changed

- Scan schema 1.1 adds protocol/hardware revisions, commanded step sizes, units,
  calibration/operator context, per-reading source/validity/warnings, and sequence
  numbers while retaining schema 1.0 reads.
- Stage 1 roadmap now represents the complete Version 1 architecture; physical
  two-axis hardware remains Stage 2.

[Unreleased]: https://github.com/bostromdev/Radiance3D
