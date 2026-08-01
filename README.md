# Radiance3D

> **Visualize the invisible.**

Radiance3D is an open-source platform for automated three-dimensional antenna radiation-pattern measurement, motion control, RF acquisition, processing, and visualization.

The project separates host applications, reusable software packages, embedded firmware, simulation, hardware documentation, and measurement data behind versioned interfaces.

> [!IMPORTANT]
> Radiance3D is being built in my house right now lol, very easy to make. ANYONE CAN DO IT. Shows the simplicity of visualizing RF waves. It has not been validated as laboratory-grade measurement equipment, and simulated or provisional results must not be represented as calibrated physical measurements(this is to avoid getting sued, wish I didn't have to put it in.)

## Current status

The current validated baseline includes:

- Native ESP-IDF firmware target
- Portable C++ controller simulator
- Python motion-controller client
- Versioned command and event protocol
- Dual-axis motion state machine
- Homing behavior
- Latched emergency stop
- Host-heartbeat timeout handling
- Driver diagnostics interfaces
- Portable firmware tests
- Python host-to-simulator integration tests

The simulator and host integration tests pass. Physical motor commissioning, RF receiver integration, calibration, and production visualization coming soon! This is a very exciting project for me.

## System architecture

```text
Host applications----------------------/\
        |                             /--\
Reusable Python packages-------------/----\
        |                           /------\
Motion client and protocol--------_/--------\
        |                     (MAC,LINUX,WINDOWS)       
   +----+------------------+                  
   |                       |   
Simulator              ESP32 firmware
                           |
                    Physical motion layer
                           |
                   TMC2209 motor drivers
                           |
                    NEMA 17 stepper motors
```

The simulator and physical firmware expose the same protocol. This allows command handling, sequencing, response correlation, timeout behavior, and recovery logic to be tested before motor power is connected.

## Repository layout

```text
Radiance3D/
├── apps/
│   ├── host-cli/
│   ├── scanner/
│   ├── simulator-ui/
│   └── visualizer/
├── packages/
│   ├── protocol/
│   ├── motion-client/
│   ├── scan-models/
│   └── processing/
├── firmware/
│   ├── controller/
│   ├── shared/
│   └── tests/
├── hardware/
│   ├── electronics/
│   ├── mechanical/
│   ├── commissioning/
│   └── reference-designs/
├── simulations/
│   ├── motion/
│   └── rf/
├── data/
│   ├── schemas/
│   ├── examples/
│   └── fixtures/
├── docs/
│   ├── architecture/
│   ├── protocol/
│   ├── hardware/
│   ├── development/
│   └── calibration/
├── tests/
├── scripts/
└── assets/
```

Existing implementation files remain in their current locations during the first structure-only phase. They will be migrated incrementally while preserving passing tests and public interfaces.

## Development workflow

1. Define protocol and data boundaries.
2. Validate host behavior against the simulator.
3. Build the ESP-IDF firmware.
4. Perform USB-only ESP32 bring-up.
5. Commission drivers, switches, and motors.
6. Integrate RF acquisition.
7. Establish repeatability and calibration procedures.
8. Implement processing and visualization.
9. Validate complete physical scans.

## Python setup

```bash
cd software
python3.11 -m venv .venv
source .venv/bin/activate
python -m pip install -e ".[dev,serial]"
```

## Build and test the simulator

```bash
cd firmware/controller
cmake -S host -B build-host -DCMAKE_BUILD_TYPE=Release
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

## Run the Python integration tests

```bash
cd software
source .venv/bin/activate

RADIANCE3D_SIMULATOR="$PWD/../firmware/controller/build-host/radiance3d-simulator" \
pytest -vv tests/test_firmware_simulator_protocol.py
```

## Build the ESP32 firmware

Radiance3D currently targets ESP-IDF v5.5.4.

```bash
source ~/esp/esp-idf/export.sh
cd firmware/controller
idf.py set-target esp32
idf.py build
idf.py size
```

Building firmware does not flash or power the physical motion system.

## Safety boundary

Simulator validation does not prove:

- Wiring correctness
- GPIO correctness
- Motor direction
- Driver current configuration
- Step-pulse timing under load
- Limit-switch polarity
- Mechanical clearance
- Physical emergency-stop latency
- RF accuracy
- Calibration quality

Physical commissioning must begin with USB power only, motor power disconnected, accessible power isolation, and supervised testing.

## Documentation

Start with:

- `docs/architecture/ARCHITECTURE.md`
- `docs/architecture/RESTRUCTURE.md`
- `docs/development/setup.md`
- `docs/hardware/tmc2209-commissioning.md`
- `ROADMAP.md`

## License

Radiance3D software and documentation are licensed under the MIT License unless otherwise stated.
