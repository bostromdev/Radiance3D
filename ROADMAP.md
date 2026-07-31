# Radiance3D Roadmap

Stages are sequential engineering targets, not release promises. A stage is complete
only when its outputs are documented and reproducible.

## Stage 0 — Repository foundation

Architecture, terminology, schemas, contribution standards, and CI. Exit when the
simulated example validates and software and firmware simulator checks pass.

## Stage 1 — Version 1 engineering architecture

Two-axis configurable motion interfaces, commanded-position confidence, homing and
safety semantics, receiver-neutral host boundaries, raster planning, synchronized
sampling, complete raw-data provenance, and simulator tests. Exit requires all
interfaces and provisional hardware assumptions to be documented without physical
performance claims.

## Stage 2 — Physical two-axis scanner

ESP32 GPIO, replaceable driver implementation, two NEMA 17 axes, switches, emergency
stop, acceleration, timeouts, and synchronized stepping. Exit requires documented
electrical safety checks, fault behavior, cable limits, and repeatable zeroing.

## Stage 3 — RF acquisition

At least one host-side receiver adapter, timestamped native measurements, raw capture,
and reference measurements. Exit requires receiver-specific limitations, retry/timeout
tests, and complete provenance.

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
