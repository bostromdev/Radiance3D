# Radiance3D

> **Visualize the invisible.**

A 3D antenna radiation-pattern measurement rig. An ESP32 drives two NEMA 17 axes
through TMC2209 drivers to point an antenna, while a stationary receiver measures it.

Built in my house. It is genuinely easy to make — anyone can do it. Built on
first-principles physics, with laboratory-grade accuracy as the goal. That has to be
earned: it needs calibration and repeatability testing that hasn't happened yet. Until
then, nothing here is a calibrated measurement.

See [LICENSE](LICENSE) for terms. All rights reserved — developed in the open, not
open-source.

## What actually works

- ESP-IDF firmware for the ESP32, compiled and unit tested
- Portable C++ controller simulator with the same protocol as the real firmware
- Python motion-control client and host integration tests
- Versioned command/event protocol, dual-axis motion state machine, homing, latched
  emergency stop, heartbeat timeout, driver diagnostics
- Versioned scan data schema

All of it passes its own tests. **None of it has driven a physical motor yet.**

## What doesn't exist yet

Physical bring-up, the mechanism, RF acquisition, calibration, visualization. There is
no CAD, no printed part, and no measured antenna pattern.

## Repository

| Path | What |
|---|---|
| `Measurements/` | Caliper measurements and source photos. Canonical for every dimension. |
| `HARDWARE.md` | Components, motor wiring, GPIO baseline, power, safety. |
| `firmware/` | ESP32 controller and simulator (C++). |
| `software/` | Host client, scan validation, CLI (Python). |
| `data/` | Scan schema and a simulated example. |
| `scripts/` | Repository checks. |

## Measurements

[`Measurements/`](Measurements/README.md) is the canonical source for every physical
dimension. Measured, photographed, and traceable — 35 photos, every value citing the
picture it came from.

Measured so far: the NEMA 17 motors (`42HDB0014NC-24B`), the TMC2209 V1.3 drivers with
heatsinks, the ELEGOO ESP32 devkit, and the AD8317 detector board.

Two rules that folder enforces:

- **Measured values are frozen.** Fit is tuned through clearance parameters, never by
  editing a measurement.
- **Nothing is guessed.** Values that can't be read confidently are marked unclear and
  listed in [`missing-measurements.md`](Measurements/missing-measurements.md).

Still missing, and blocking the CAD: bearings, shaft couplers, fasteners, the antenna
mount, and the NEMA 17 mounting-hole pattern.

## CAD

[`fusion360-parameters.md`](Measurements/fusion360-parameters.md) is the parameter
table. [`fusion360-assistant-prompt.md`](Measurements/fusion360-assistant-prompt.md) is
a paste-ready prompt for Fusion Assistant.

Parameters are prefixed by type: `m` measured, `c` calculated, `d` design choice,
`p` provisional. Printing in PETG.

## Build and test

```bash
# Python host
cd software
python3.11 -m venv .venv && source .venv/bin/activate
python -m pip install -e ".[dev,serial]"
pytest -vv

# Controller simulator
cd firmware/controller
cmake -S host -B build-host -DCMAKE_BUILD_TYPE=Release
cmake --build build-host
ctest --test-dir build-host --output-on-failure

# ESP32 firmware (ESP-IDF v5.5.4)
source ~/esp/esp-idf/export.sh
cd firmware/controller
idf.py set-target esp32 && idf.py build
```

Building firmware does not flash or power anything.

## Safety

Commissioning starts with USB power only, motor power disconnected, and power isolation
that does not depend on firmware. Motion systems pinch and move unexpectedly. RF
transmission must comply with local law and licensing.
