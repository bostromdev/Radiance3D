# RF measurement architecture

The antenna under test rotates while the RF source or measurement reference and
receiver remain stationary. The receiver connects to the host through a
device-neutral `MeasurementAdapter`; it is logically and electrically separate from
the ESP32 motion controller. Version 1 defines this interface but implements no
RX5808, SDR, spectrum-analyzer, power-detector, NanoVNA-derived, or custom-detector
support.

Each adapter returns a numeric native value, unit, timezone-aware timestamp, stable
source identifier, validity state, and optional warnings. Conceptual units include
dBm, relative dB, volts, ADC counts, RSSI units, and arbitrary units. Arbitrary
receiver output must not be relabeled dBm without a documented calibration model.

The RF source and AUT arrangement must be chosen for the measurement method and local
legal requirements. Record fixed separation, repeatable antenna height, polarization,
clear line of sight, support material, source-power stability, consistent coax routing,
and whether sufficient far-field distance was practical.

Feedline movement, common-mode current, receiver overload, reflections, nearby
conductive structure, motor wiring, and controller emissions can distort patterns.
Cable routing and non-conductive structural materials should reduce disturbance where
testing shows benefit. Reduce nearby reflective surfaces and preserve environment
notes. No receiver, dynamic range, accuracy, supported frequency range, or true-gain
measurement is currently validated.
