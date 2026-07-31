# Radiance3D Architecture

## Purpose

This document defines the intended boundaries between Radiance3D applications, reusable packages, firmware, simulation, hardware, and measurement data.

## Core rules

1. The protocol must have one authoritative specification.
2. Host applications must use public client interfaces.
3. Simulator and physical firmware behavior must remain compatible.
4. Hardware-specific code must remain separate from portable controller logic.
5. Scan models and processing must not depend directly on serial transport.
6. Simulated, provisional, measured, and calibrated data must remain distinguishable.
7. Structural changes must not silently alter safety or motion behavior.

## Dependency direction

```text
Applications
    |
Reusable packages
    |
Client interfaces
    |
Protocol and transport
    |
Simulator or ESP32 firmware
    |
Physical hardware
```

Dependencies should flow downward. Lower layers must not import user-interface or application code.

## Applications

Applications provide complete user workflows without owning core protocol or data logic.

Planned applications include:

- Host command-line interface
- Scan coordinator
- Simulator interface
- Dataset inspector
- Two-dimensional plot viewer
- Interactive three-dimensional visualizer

## Reusable packages

### Protocol

Owns:

- Command names
- Response formats
- Event formats
- Error codes
- Correlation rules
- Protocol versions
- Compatibility requirements

### Motion client

Owns the host-facing API for:

- Connection
- Identification
- Capabilities
- Configuration
- Homing
- Movement
- Status
- Diagnostics
- Stop
- Emergency stop
- Fault recovery

### Scan models

Owns versioned representations for:

- Scan metadata
- Position samples
- Receiver samples
- Calibration metadata
- Warnings
- Provenance
- Processing history

### Processing

Owns operations such as:

- Normalization
- Reference correction
- Coordinate conversion
- Interpolation
- Beamwidth estimation
- Front-to-back ratio
- Side-lobe analysis
- Dataset comparison

## Firmware

### Portable core

The portable core contains behavior that can run on a development computer:

- Protocol parsing
- Controller state
- Command correlation
- Motion state transitions
- Safety state transitions
- Simulator behavior

### ESP32 integration

The ESP32 layer owns:

- ESP-IDF startup
- GPIO configuration
- UART configuration
- TMC2209 communication
- Hardware timers
- Step generation
- Limit-switch input
- Watchdog behavior
- Physical diagnostics

### Shared firmware boundary

Shared firmware definitions may be consumed by portable and ESP32 builds, but portable builds must not accidentally depend on ESP-IDF.

## Simulation

### Motion simulation

The motion simulator validates externally observable controller behavior:

- Command flow
- Response correlation
- Completion events
- Fault events
- Emergency-stop behavior
- Heartbeat timeout behavior
- Recovery sequencing

It does not validate electronics or mechanics.

### RF simulation

Synthetic RF data supports:

- Schema development
- Processing tests
- Plotting tests
- Visualization work
- Regression tests

Synthetic data must remain clearly labeled and must never be represented as physical measurement evidence.

## Data flow

```text
Position command
      |
Motion controller
      |
Position report
      |
Host scan coordinator
      |
Receiver sample
      |
Versioned scan record
      |
Validation
      |
Calibration and processing
      |
Visualization
```

Each sample should preserve:

- Requested position
- Reported position
- Position confidence
- Receiver value
- Timestamp
- Frequency
- Unit
- Calibration state
- Software version
- Hardware configuration

## Safety invariants

Where applicable, simulator and physical firmware must preserve these behaviors:

- Emergency stop remains latched.
- Motion is rejected while emergency stop is active.
- Fault recovery requires an explicit command.
- Host-heartbeat loss disables active motion.
- Position trust is removed when physical position cannot be guaranteed.
- Motion events correlate with command identifiers.
- Invalid commands fail safely.
