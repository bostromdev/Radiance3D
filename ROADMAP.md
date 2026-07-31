# Radiance3D Roadmap

Stages are sequential engineering targets, not release promises. A stage is complete
only when its outputs are documented and reproducible.

## Stage 0 — Repository foundation

Architecture, terminology, schemas, contribution standards, and CI. Exit when the
simulated example validates and software and firmware simulator checks pass.

## Stage 1 — Motion-control prototype

Single-axis movement, repeatable positioning, command protocol, and simulator mode.
Exit requires documented test conditions and repeatability results.

## Stage 2 — Two-axis scanner

Azimuth and elevation, homing, safety limits, and synchronized stepping. Exit
requires safe fault behavior and repeatable zeroing.

## Stage 3 — RF acquisition

Receiver integration, timestamped measurements, raw capture, and reference
measurements. Exit requires receiver-specific limitations and provenance.

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
