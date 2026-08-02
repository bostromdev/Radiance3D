# TMC2209 and NEMA 17 commissioning

> [!CAUTION]
> This document records the Version 1 hardware baseline and the pending validation
> steps. It does not claim physical testing. Never connect or disconnect a stepper
> motor while its driver is powered. Provide a physical motor-power disconnect; a
> software emergency stop is not a substitute.

## Implemented hardware baseline

Version 1 uses an ESP32 development board with an ESP-WROOM-32 module, USB-C, 3.3 V
logic, and USB serial for host communication. The motion system uses two
BIGTREETECH TMC2209 V1.3 drivers. The motors are YEJMKJ/LYLANMO NEMA 17 bipolar
4-wire units with 1.8° full-step geometry and a 1.0 A rated phase current. The exact
ESP32 board revision, TMC2209 carrier revision, UART wiring, R10 setting, sense
resistor, and pinout remain pending validation.

Native ESP-IDF startup sets STEP low, keeps the enable pins inactive, validates the
generated profile, configures GPIO/UART/GPTimer resources, probes each driver, reads
diagnostics, writes current/microstep settings, reads switches and the e-stop input,
and leaves both axes disabled and untrusted. A missing driver is a structured fault and
cannot be enabled.

The firmware follows the TMC2209 datagram/CRC/register definition, bounds UART reads,
filters expected write echo, and verifies register writes with IFCNT. STEP timing
remains conservative and must be measured on hardware before final claims are made. See the
[Analog Devices TMC2209 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/TMC2209_datasheet_rev1.09.pdf).

## Version 1 hardware profile

The current profile is intended for the selected hardware family and uses the following
baseline values:

- Controller: ESP32 development board, ESP-WROOM-32, USB serial, 3.3 V logic
- Drivers: BIGTREETECH TMC2209 V1.3, UART controlled, STEP/DIR interface,
  diagnostic reporting enabled, current configured in RMS milliamps
- Motors: NEMA 17 bipolar, 4-wire, model `42HDB0014NC-24B`, 1.8° full-step, 200 full
  steps/revolution, 1.0 A rated phase current, 3.5 Ω phase resistance, 3.4 mH phase
  inductance, approximately 0.13 N·m holding torque. Coil A is black + green, coil B
  is red + blue — see [wiring](wiring.md). Verify the pairs with a multimeter before
  connecting a motor: about 3.5 Ω within a pair, open circuit between pairs

The GPIO assignments below are a working baseline for the current firmware but remain
provisional until the exact board revision and carrier label map are confirmed.

| ESP32/TMC signal | Azimuth | Elevation | Level | Purpose | Status |
| --- | ---: | ---: | --- | --- | --- |
| STEP → TMC2209 STEP | GPIO25 | GPIO18 | 3.3 V logic | Step edge | Baseline, verify against board |
| DIR → TMC2209 DIR | GPIO26 | GPIO19 | 3.3 V logic | Direction | Baseline, verify against board |
| ENABLE → TMC2209 EN/ENN | GPIO27 | GPIO23 | 3.3 V logic, active low | Output enable | Confirm carrier polarity |
| UART TX → PDN_UART | GPIO22 / UART1 | GPIO17 / UART2 | 3.3 V logic | Configuration/write | Confirm carrier interface |
| UART RX ← PDN_UART | GPIO21 / UART1 | GPIO16 / UART2 | 3.3 V logic | Diagnostics/read | Confirm one-wire circuit |
| Home switch | GPIO32 | GPIO33 | 3.3 V input/pull-up | Homing and limit detection | Configurable |
| Emergency-stop sense | GPIO13 | shared | 3.3 V input/pull-up | Active-low input | Pending validation |
| Logic ground → GND | common | common | 0 V reference | STEP/DIR/UART reference | Required |
| Motor-power ground → GND | common | common | 12 V return | VM return tied to common ground | Required |
| VM | 12 V supply | 12 V supply | 12 V power | Motor driver supply only | Required; never connect to ESP32 |

The actual carrier may expose `PDN_UART`, `UART`, `CFG`, `MS1/MS2`, `SPREAD`, or
other labels and may require a resistor for bidirectional UART. Resolve those details
from the exact carrier schematic before wiring the final build.

## Power, motor wiring, and current

Distribute 12 V separately to both VM inputs and a buck converter. Feed the ESP32 only
through the input method documented for the exact board. All logic references share
ground, but keep high motor-current returns short and away from switch/RF returns.
Place appropriately rated bulk capacitance close to each driver's VM/GND and local
logic decoupling near the electronics. Review the board's USB/external-power circuit
before connecting USB and the buck at the same time.

Identify the motor's two coil pairs and connect each pair to one driver phase. A 12 V
supply does not mean the driver continuously applies 12 V across a winding; the
TMC2209 chops the supply to regulate winding current. Incorrect RMS current can still
overheat or under-drive the motor/driver.

The initial commissioning current is 650 mA RMS. The motor ceiling is 1000 mA RMS.
Suggested commissioning progression is:

500–650 mA → 650–800 mA → approximately 900 mA only if necessary → never exceed
rated phase current

Final current must be verified using actual torque requirements, motor temperature,
driver temperature, and long-duration testing. Reduced hold current is configured as
30% for azimuth and 40% for elevation in the current provisional profile.

## Homing and position trust

Each axis accepts normally-closed or normally-open switch logic. The default is
normally closed with pull-up, but the polarity remains configurable. Homing validates
an inactive switch, approaches quickly, stops on a debounced activation, backs off,
confirms release, approaches slowly, stops, applies the configured offset, and only
then marks position trusted.

The firmware distinguishes commanded position, estimated position, trusted position,
and physical position. Commanded position is the open-loop step count. Estimated
position is the internal model. Trusted position is the state after a successful home.
Physical position is the real-world location to be measured externally. Trust is lost on
reset, fault, emergency stop, driver disable, timeout, motion interruption, or any
suspicion of missed steps.

## First power-up checklist

1. Verify the buck converter output with a multimeter and set it to exactly 5.0 V before connecting the ESP32.
2. Verify polarity and common ground between the 12 V rail, the TMC2209 logic reference, and the buck output.
3. Verify the inline fuse is installed and the master disconnect is accessible.
4. Verify the drivers are disabled before power is applied.
5. Verify UART communication and confirm the driver responds over the configured channel.
6. Connect one motor only.
7. Configure 650 mA RMS current.
8. Test a 20-step movement and verify the direction.
9. Verify temperatures on the motor, driver, and buck converter.
10. Test the home switch.
11. Home the first axis.
12. Repeat for the second axis.
13. Test coordinated motion.

Warnings:

- A car battery can supply very large current and can short dangerously.
- USB and external power can backfeed into the logic rail if the board topology is not reviewed.
- Hot-plugging motors can cause transients and driver faults.
- Incorrect current settings can overheat the motor and driver.

## Fault interpretation

| Fault/status | Required response |
| --- | --- |
| Driver communication / absent | Keep axis disabled; verify UART topology, channel, common ground, and logic level. |
| Reset detected / undervoltage | Stop; inspect VM and logic supply sequencing, wiring, bulk capacitance, and transients. |
| Overtemperature warning | Stop the test soon; reduce current/load or improve cooling and measure temperature. |
| Overtemperature shutdown | Driver is disabled; remove motor power, allow cooling, diagnose before reset, and rehome. |
| Short to ground/supply | Remove power immediately and inspect motor cable, connector, phase pairing, and carrier. |
| Homing stuck / never / release / timeout | Remove motor power if travel is unsafe; inspect switch polarity, mechanics, wiring, direction, debounce, speed, and travel. |
| Unexpected home switch | Stop and rehome only after checking limits, direction, switch noise, and mechanical position. |
| Host heartbeat timeout | Drivers have been stopped/disabled; inspect USB/host failure and rehome because position is untrusted. |
| Emergency stop | Use the physical disconnect as necessary; clear the hazard, release the input, explicitly reset, and rehome. |

## Known limitations and required confirmations

- Exact ESP32 board and TMC2209 carrier revisions/pinouts are pending confirmation.
- Carrier UART one-wire circuitry, address straps, sense resistance, enable polarity,
  and onboard potentiometer interaction are pending confirmation.
- Motor rated current, thermal limits, and load behavior remain to be validated.
- No encoder, stall verification, backlash compensation, or continuous-rotation support
  exists in Version 1.
- USB backfeed behavior, power transients, thermal performance, homing repeatability,
  cable clearance, and coordinated physical motion remain unvalidated.
