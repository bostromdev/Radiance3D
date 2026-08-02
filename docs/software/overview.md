# Software overview

The initial `radiance3d` package provides typed domain models, core version 1
validation, and scan inspection. Acquisition, analysis, calibration, and visualization
interfaces are architectural boundaries only; there is no GUI or hardware control.

The package uses a `src` layout and Python 3.11+. The JSON Schema is the normative
interchange contract; Python validation gives fast, dependency-free feedback for core
invariants.
