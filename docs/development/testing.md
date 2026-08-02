# Testing

- **Repository:** required paths, valid JSON, simulated provenance, and internal links.
- **Software:** model invariants, CLI results, formatting, lint, and strict type checks.
- **Firmware:** portable CMake/CTest simulator and core tests plus native ESP-IDF
  compilation of the provisional ESP32 target. Tests cover UART framing/write echo,
  current ceilings, diagnostics, rational conversion, motion, homing, safety,
  dual-axis completion, generated-profile consistency, and protocol contracts.
- **Host/firmware integration:** the unchanged Python serial client runs against the
  compiled native simulator in CI, including identity, command correlation, and
  heartbeat behavior.
- **Schemas:** metaschema check plus example validation.
- **Hardware:** future test records must identify exact revisions, setup, instruments,
  raw data, and safety controls.

A passing portable test or ESP32 compilation is not evidence that the selected board,
carrier, motor, switch, power supply, thermal design, or mechanics work physically.
