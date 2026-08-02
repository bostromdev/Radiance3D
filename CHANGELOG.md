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
- Physical ESP32 motion layer with TMC2209 UART diagnostics and current control,
  non-blocking dual-axis STEP/DIR motion, two-pass switch homing, and latched
  emergency-stop behavior.
- Correlated motor protocol commands, configuration/state inspection, completion and
  fault events, heartbeat timeout handling, and an optional pyserial host transport.
- Stage 2 simulator fault injection, native/host tests, provisional GPIO/current
  configuration, and a safety-focused commissioning guide.
- `Measurements/` as the canonical source for physical dimensions, with caliper
  measurements and source photographs for the NEMA 17 motors, the BIGTREETECH
  TMC2209 V1.3 drivers, and the ELEGOO ESP32 devkit.
- `Measurements/missing-measurements.md` recording every unclear or unmeasured
  dimension, why it matters, and how to take it.
- Fusion 360 handoff: a user-parameter table separating measured, design-choice,
  provisional and calculated values, and a complete pan-and-tilt prototype prompt.

### Changed

- Scan schema 1.1 adds protocol/hardware revisions, commanded step sizes, units,
  calibration/operator context, per-reading source/validity/warnings, and sequence
  numbers while retaining schema 1.0 reads.
- Stage 2 is implemented and compile/unit-test validated; physical commissioning and
  thermal/mechanical validation remain pending.
- Repository restored to a single project root. Commit `e3d297e` had reduced the
  tracked tree to `LICENSE` and `README.md`; the project content has been returned to
  the root and committed. No history was rewritten.
- Hardware, mechanical, CAD, BOM, architecture and index documentation now link to
  `Measurements/` instead of restating dimensions, so there is one place to correct.
- Roadmap trimmed to remaining work; Stages 0 and 1 recorded as complete.

[Unreleased]: https://github.com/bostromdev/Radiance3D
