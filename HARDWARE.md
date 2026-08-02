# Hardware

Verified facts only. Physical dimensions live in
[`Measurements/`](Measurements/README.md) and are not repeated here.

## Components

| Part | Model | Qty |
|---|---|---:|
| Stepper motor | YEJMKJ / LYLANMO `42HDB0014NC-24B` | 2 |
| Stepper driver | BIGTREETECH TMC2209 V1.3 | 2 |
| Controller | ELEGOO ESP32 devkit, ESP-WROOM-32, USB-C | 1 |
| RF detector | AD8317 EVAL BD (NWDZ V1.0) | 1 |
| Buck converter | LM2596-based module | 1 |
| Motor supply | 12 V battery, inline fuse, master disconnect | 1 |

## Motor nameplate

From the label, legible in `IMG_5249.HEIC`:

| | |
|---|---|
| Holding torque | 0.13 N·m |
| Phase resistance | 3.5 Ω |
| Phase inductance | 3.4 mH |
| Rated phase current | 1.0 A |
| Step angle | 1.8° (200 full steps/rev) |
| Frame | 42 × 42 × 21 mm |

## Motor phase wiring

| Wire | Phase |
|---|---|
| Black | A+ |
| Green | A− |
| Red | B+ |
| Blue | B− |

**Coil A = black + green. Coil B = red + blue.** Splitting a pair across the two driver
outputs makes the motor buzz, jitter or lock instead of turning, and it is not obvious
by ear.

Check with a multimeter before energising: about 3.5 Ω within a pair, open circuit
between pairs.

Never connect or disconnect a motor while the drivers are powered.

## GPIO baseline

Working baseline for the current firmware. Verify against the actual board before
final wiring.

| Signal | Azimuth | Elevation | Notes |
|---|---|---|---|
| STEP | GPIO25 | GPIO18 | 3.3 V logic |
| DIR | GPIO26 | GPIO19 | 3.3 V logic |
| ENABLE (EN/ENN) | GPIO27 | GPIO23 | Active low — confirm carrier polarity |
| UART TX → PDN_UART | GPIO22 / UART1 | GPIO17 / UART2 | One-wire; may need a resistor |
| UART RX ← PDN_UART | GPIO21 / UART1 | GPIO16 / UART2 | |
| Home switch | GPIO32 | GPIO33 | Configurable; see homing note below |
| Emergency stop | GPIO13 | shared | Active low |

Logic ground and motor-power ground must share a common reference. VM is 12 V to the
drivers only — never to the ESP32.

## Power

```text
12 V battery
├── inline fuse (3–5 A)
├── master disconnect
├── TMC2209 VM ×2
└── LM2596 buck → 5.0 V → ESP32 only
```

Set the buck output to exactly 5.0 V with a multimeter before connecting an ESP32. Do
not trust an onboard display as the only check.

Bulk capacitance (100–220 µF, 25 V minimum) close to each driver's VM/GND, plus 0.1 µF
local decoupling. Steppers regenerate current when decelerating, so VM transients
appear even when the command is static.

Keep motor-current returns short and away from logic, switch and RF returns.

USB and the buck rail can both energise the board. Check the board's power circuit
before connecting both at once.

## Motor current

Commissioning starts at **650 mA RMS**. Motor ceiling is **1000 mA RMS**.

Progression: 500–650 → 650–800 → ~900 only if needed. Never exceed the ceiling. A 12 V
supply does not put 12 V across a winding — the TMC2209 chops to regulate current — but
wrong RMS current still overheats or under-drives the motor.

## Homing

The firmware currently homes by driving to a switch (`home_switch_pin`). There is no
sensorless/StallGuard implementation. Three options if switches are not wanted:

1. **Skip homing.** Set zero by hand. Works now, no parts, no code. Zero moves on every
   power cycle, so scans are not comparable to each other.
2. **StallGuard sensorless homing.** The TMC2209 supports it and no switch is needed,
   but it is not written yet and needs mechanical hard stops. A light antenna is the
   hard case for stall detection.
3. **One microswitch per axis.** Firmware already supports it. Most repeatable.

Position is open-loop step counting. It is only trusted after a successful home. Reset,
fault, e-stop or a missed step invalidates it.

## Safety

Physical commissioning starts with USB power only, motor power disconnected, and
accessible power isolation that does not depend on firmware.

Simulator tests prove none of: wiring correctness, GPIO correctness, motor direction,
driver current, step timing under load, switch polarity, mechanical clearance,
e-stop latency, RF accuracy.
