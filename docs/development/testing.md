# Testing

- **Repository:** required paths, valid JSON, simulated provenance, and internal links.
- **Software:** model invariants, CLI results, formatting, lint, and strict type checks.
- **Firmware:** native simulator build/tests plus compilation of the provisional
  ESP32 target. Native tests cover UART framing, current limits, diagnostics, integer
  conversion, motion, homing, safety, dual-axis completion, and protocol contracts.
- **Schemas:** metaschema check plus example validation.
- **Hardware:** future test records must identify exact revisions, setup, instruments,
  raw data, and safety controls.

A passing native test or ESP32 compilation is not evidence that the selected board,
carrier, motor, switch, power supply, thermal design, or mechanics work physically.
