# Glossary

- **AUT:** Antenna under test.
- **Azimuth:** Horizontal angle in degrees: 0° forward, 90° right, 180° rear, 270° left.
- **Elevation:** Vertical angle in degrees: 0° horizon, +90° up, -90° down.
- **Sample:** One synchronized angle and RF measurement.
- **Scan:** A collection of samples made under one configuration.
- **Dataset:** Stored scan data plus metadata, warnings, and provenance.
- **Calibration:** Correction based on a known process or reference.
- **Simulated data:** Generated data not captured by physical hardware.
- **Measured data:** Values captured from a physical receiver during an experiment.
- **Imported data:** Data translated from an external format without changing its
  underlying meaning.
- **Processed data:** Derived data whose provenance identifies its source datasets and
  method.
- **Commanded position:** Position calculated from open-loop step commands; not an
  independent physical observation.
- **Position confidence:** Controller state indicating whether commanded position is
  usable after a successful home and has not been invalidated by reset, fault, stop,
  emergency stop, disable, timeout, or suspected missed steps.
- **Commanded angular resolution:** Smallest configurable motion increment; not the
  same as mechanical resolution, repeatability, or absolute accuracy.
