# Measurement procedure

No physical procedure is validated. A Version 1 experiment record should document:

1. legal and safety review;
2. exact equipment, firmware, software, protocol, and hardware revisions;
3. motor current, warm-up, airflow, power, motion, and receiver configuration;
4. AUT identity and forward/right/up coordinate orientation;
5. fixed separation, repeatable height, polarization, line of sight, nearby
   reflections, non-conductive supports, far-field assessment, and feedline routing;
6. RF source frequency/power stability and legal operating conditions;
7. homing result, configured limits, angular sequence, acceleration, velocity,
   settling time, samples/position, averaging, timeout, retry, and receiver settings;
8. reference measurements, calibration status, operator/environmental notes;
9. raw accepted and rejected capture with position kind, sequence, timestamp, source,
   native unit, validity, warnings, and provenance; and
10. shutdown plus checks for invalid/missing samples, cable winding, driver
    temperature, suspected missed steps, and any loss of position confidence.

Do not claim true gain unless an appropriate reference method and uncertainty budget
support it. A calibrated receiver alone does not calibrate chamber geometry,
reflections, feedline radiation, polarization, or angular position.
