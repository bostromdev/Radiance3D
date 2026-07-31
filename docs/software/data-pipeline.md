# Data pipeline

The planned pipeline preserves rather than overwrites raw observations:

1. Acquire receiver-native values with units and timestamps.
2. Pair them with reported angles and quality flags.
3. Store a `measured` raw dataset with full configuration.
4. Apply calibration or normalization as a named transformation.
5. Store a new `processed` dataset referencing its source IDs.
6. Interpolate or convert coordinates only in explicit later stages.
7. Render the selected dataset and expose processing status.

Algorithms must handle missing, irregular, and repeated angles. They must not assume
all receiver values are logarithmic, RSSI, power, or directly comparable.
