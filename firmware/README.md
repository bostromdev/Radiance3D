# Firmware

`controller/` defines a versioned, line-oriented motion protocol and a buildable
simulator target. It establishes interfaces for azimuth, elevation, homing, limits,
position reports, scan-step synchronization, stop, and faults.

No motor driver, pinout, controller board, or physical safety behavior is verified.
Hardware-specific implementations must preserve the protocol contract and add
independent safety controls before energizing motors.
