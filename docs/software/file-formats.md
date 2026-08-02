# File formats

Scan records use the versioned JSON schema in `data/schemas/scan-v1.schema.json`.
Simulated data is explicitly labelled. AD8317 acquisition records must distinguish raw
ADC counts and volts from any calibration-derived RF-power estimate.
