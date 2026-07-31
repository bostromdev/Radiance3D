# Data pipeline

The planned pipeline preserves rather than overwrites raw observations:

1. Move to a bounds-checked raster point and wait for trusted motion completion.
2. Apply the configured settling time.
3. Acquire receiver-native values with units, timestamps, source, validity, warnings,
   timeout, and retry policy.
4. Pair every accepted or rejected raw reading with commanded-position kind,
   sequence number, and quality flags.
5. Store a `measured` raw dataset with full configuration before processing.
6. If requested, average only as an additional result or named derived dataset; raw
   individual readings remain available.
7. Apply calibration or normalization as a named transformation.
8. Store a new `processed` dataset referencing its source IDs.
9. Interpolate, convert coordinates, prepare visualization, or export only in explicit
   later stages.

Algorithms must handle missing, irregular, and repeated angles. They must not assume
all receiver values are logarithmic, RSSI, power, or directly comparable.

The dependency-free host package now provides `MotionController` and
`MeasurementAdapter` protocols, a limits-aware raster planner, and a scan coordinator
for move/settle/measure behavior. No serial transport or physical receiver adapter is
implemented yet.
