# Radiance3D Roadmap

Stages are sequential engineering targets, not release promises. A stage is complete
only when its outputs are documented and reproducible.

This page lists remaining work. Completed stages are recorded briefly so the sequence
still reads, but they carry no outstanding tasks.

## Completed

- **Stage 0 — Repository foundation.** Architecture, terminology, schemas,
  contribution standards, and CI are in place. The simulated example validates and the
  software and firmware simulator checks pass.
- **Stage 1 — Version 1 engineering architecture.** Two-axis configurable motion
  interfaces, commanded-position confidence, homing and safety semantics,
  receiver-neutral host boundaries, raster planning, synchronized sampling, raw-data
  provenance, and simulator tests are documented in
  [`docs/architecture/version-1.md`](docs/architecture/version-1.md) without physical
  performance claims.

## Stage 2 — Physical two-axis scanner (in progress)

Firmware, host protocol, simulator parity, tests, and commissioning documentation are
complete. Hardware dimensions for the ESP32, both TMC2209 drivers and both NEMA 17
motors are measured, and the CAD handoff is prepared.

Remaining:

- Physical bring-up on connected hardware, USB power first, motor power disconnected.
- Exact-board pin confirmation, UART wiring, R10 and sense-resistor verification.
- Thermal characterization under load.
- Repeatable homing and zeroing evidence.
- Cable-envelope evidence through full travel.
- Documented electrical safety checks and fault behavior.
- Complete the outstanding physical measurements in
  [`Measurements/missing-measurements.md`](Measurements/missing-measurements.md).
  Bearings, shaft couplers, fasteners, the antenna mount and the limit switches have
  no measurements at all, and they block the mechanical design.
- Design, print and fit-test the pan-and-tilt mechanism. Start from
  [`Measurements/fusion360-assistant-prompt.md`](Measurements/fusion360-assistant-prompt.md),
  and print the fit-test coupons before committing any part that depends on a
  parameter marked provisional.

Exit requires documented electrical safety checks, fault behavior, cable limits, and
repeatable zeroing.

## Stage 3 — RF acquisition

At least one host-side receiver adapter, timestamped native measurements, raw capture,
and reference measurements. Exit requires receiver-specific limitations, retry/timeout
tests, and complete provenance.

No RF detector has been selected or measured.

## Stage 4 — Data processing

Normalization, interpolation, coordinate conversion, and repeatability analysis.
Exit requires tests against transparent reference cases.

## Stage 5 — Visualization

Polar plots, spherical data representation, interactive 3D visualization, and
dataset comparison. Exit requires faithful handling of missing and irregular data.

## Stage 6 — Calibration and validation

Known reference antenna, repeatability testing, uncertainty documentation, and
comparison with trusted equipment. This is the first stage that may support
evidence-bounded accuracy statements.

## Stage 7 — Public hardware release

Tested BOM, assembly instructions, verified wiring, reproducible sample datasets,
and release documentation. Hardware licensing will be selected when verified design
files are published.
