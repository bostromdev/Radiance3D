# TMC2209 and NEMA 17 commissioning

> [!CAUTION]
> This is a provisional development configuration, not verified final wiring.
> Never connect or disconnect a stepper motor while its driver is powered. Provide a
> physical motor-power disconnect; software emergency stop is not a substitute.

## Implemented architecture

One ESP32 services two independent TMC2209 drivers. Each uses a separate ESP32
hardware UART plus STEP, DIR, and active-low enable. Separate UART channels avoid
shared-bus/address ambiguity during first bring-up; both provisional addresses are
zero because they are on different channels. Confirm each carrier's PDN_UART and
address-strap implementation against its schematic before wiring.

Startup sets STEP low and both enable pins inactive, validates pins/configuration,
starts the two UARTs, probes each driver, reads diagnostics, writes conservative
current/microstep/mode settings, reads switches/e-stop, and leaves both axes disabled
and untrusted. A missing driver is a structured fault and cannot be enabled.

The internal driver follows the manufacturer datagram/CRC and register definitions;
successful writes are checked through IFCNT. STEP timing is a conservative 2 µs high,
2 µs minimum low, with 2 µs after DIR changes. These values exceed the TMC2209's
published STEP/DIR minimum timing. See the
[Analog Devices TMC2209 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/TMC2209_datasheet_rev1.09.pdf).

No large motion library was introduced. The small local implementation keeps pulse
timing, dual-axis scheduling, stop behavior, UART diagnostics, and native simulation
under direct test.

## Provisional GPIO and signal table

These assignments target PlatformIO's generic classic `esp32dev`. They avoid the
classic ESP32 bootstrapping and input-only pins, but the exact user's board and carrier
pin labels are unknown. Change the unified configuration before connecting a board
whose schematic differs.

| ESP32/TMC signal | Azimuth | Elevation | Level | Purpose | Status |
| --- | ---: | ---: | --- | --- | --- |
| STEP → TMC2209 STEP | GPIO25 | GPIO18 | 3.3 V logic | Step edge | Provisional |
| DIR → TMC2209 DIR | GPIO26 | GPIO19 | 3.3 V logic | Direction | Provisional |
| ENABLE → TMC2209 EN/ENN | GPIO27 | GPIO23 | 3.3 V logic, active low | Output enable | Provisional; confirm carrier polarity |
| UART TX → PDN_UART | GPIO22 / UART1 | GPIO17 / UART2 | 3.3 V logic | Configuration/write | Provisional; carrier interface required |
| UART RX ← PDN_UART | GPIO21 / UART1 | GPIO16 / UART2 | 3.3 V logic | Diagnostics/read | Provisional; confirm one-wire circuit |
| Driver address straps | address 0 | address 0 | Carrier-defined | UART slave address | Separate UARTs; confirm MS1/MS2 straps |
| Home switch | GPIO32 | GPIO33 | 3.3 V input/pull-up | Homing and unexpected-limit detection | Provisional, normally closed |
| Emergency-stop sense | GPIO13 | shared | 3.3 V input/pull-up | Active-low latched input | Optional/provisional |
| Logic ground → GND | common | common | 0 V reference | STEP/DIR/UART reference | Required |
| Motor-power ground → GND | common | common | 12 V return | VM return tied to common ground | Required; manage high-current return path |
| VM | 12 V supply | 12 V supply | 12 V power | Motor driver supply only | Required; never connect to ESP32 |

The actual carrier may expose `PDN_UART`, `UART`, `CFG`, `MS1/MS2`, `SPREAD`, or
different labels and may require a resistor for bidirectional single-wire UART.
Resolve those details from the exact carrier schematic. Do not infer them from this
generic table.

## Power, motor wiring, and current

Distribute 12 V separately to both VM inputs and a buck converter. Feed the ESP32 only
through the input method documented for the exact board. All logic references share
ground, but keep high motor-current returns short and away from switch/RF returns.
Place appropriately rated bulk capacitance close to each driver's VM/GND and local
logic decoupling near the electronics. Review the board's USB/external-power circuit
before connecting USB and the buck at the same time; do not assume a 5 V header is
backfeed-safe.

Identify the motor's two coil pairs with its datasheet or an unpowered continuity
test, then connect each pair to one driver phase. A 12 V supply does not mean the
driver continuously applies 12 V across a winding: the TMC2209 chops the supply to
regulate winding current. Incorrect RMS current can still overheat or under-drive the
motor/driver.

The checked-in 400 mA RMS and 800 mA ceiling are placeholders, not safe-current
claims. Before power-up, derive the setting from:

- the exact motor's rated phase current;
- the exact carrier's sense-resistor value and circuit;
- the driver's current formula and UART configuration;
- available heatsinking/airflow and mechanical load; and
- measured motor and carrier temperature during progressively longer tests.

Reduced hold current is configured as 30% while stationary. Overtemperature warning
is reported; shutdown, shorts, undervoltage/reset, or communication loss disable the
affected axis. Open-load flags are diagnostic hints and can be unreliable at
standstill or low current; do not use them as the only continuity test.

## Homing and position trust

Each axis accepts normally closed or normally open polarity. The default is normally
closed with pull-up. Homing validates an inactive switch, approaches quickly, stops
on a debounced activation, backs off, confirms release, approaches slowly, stops,
applies the configured offset, and only then marks position trusted.

Failures are distinct: switch active at start, never activated, failed to release,
overall timeout, unexpected activation during normal motion, or e-stop interruption.
Failure stops and disables the axis, leaves position untrusted, and requires operator
inspection and fault reset before retry. Open-loop microsteps are commanded position,
not verified physical accuracy; backlash, compliance, stalls, and manual movement are
not measured.

## First power-up checklist

1. Disconnect both motors and remove 12 V motor power.
2. Confirm the exact ESP32 and both carrier part numbers, schematics, pin labels,
   sense resistors, enable polarity, UART circuit, address straps, and voltage levels.
3. Inspect and edit the provisional GPIO/current/configuration record.
4. Verify the buck output with a multimeter before connecting the ESP32.
5. Verify the intended ESP32 supply pin voltage and USB backfeed protection.
6. Verify common ground and continuity from ESP32 to both driver logic grounds.
7. Verify no short between VM and ground and check all supply polarities.
8. Fit appropriately rated VM bulk capacitors and local logic decoupling.
9. Power only the ESP32/driver logic; confirm startup reports drivers disabled.
10. Run `CMD 1 MOTOR IDENTIFY` and confirm both drivers present over UART.
11. Run diagnostics for each axis; resolve communication, reset, undervoltage, short,
    or thermal faults before continuing.
12. Remove every power source, identify one motor's coil pairs, and connect only the
    azimuth motor.
13. Set an RMS current justified by that motor and carrier; do not assume 400 mA is
    correct.
14. Raise/support the mechanism so this axis can move freely, clear people/cables,
    and make the physical 12 V disconnect reachable.
15. Apply 12 V and enable only azimuth.
16. Issue exactly `CMD 10 MOTOR STEP AZ 20`; be ready to cut motor power.
17. Verify movement direction and that the reported position is explicitly untrusted.
18. Stop, disable, remove power, and inspect connector, motor, carrier, capacitor,
    buck, and wiring temperature/condition.
19. Manually actuate the unpowered home switch and verify electrical polarity/status.
20. Re-power and perform low-speed azimuth homing while ready to disconnect power.
21. Repeat steps 12–20 for elevation only.
22. Route and strain-relieve all cables through the full envelope with power removed.
23. Home both axes independently, then issue a small coordinated target within limits.
24. Confirm completion occurs after the last axis stops and only then check settling.
25. Expand distance, speed, acceleration, current, and test duration incrementally,
    recording temperatures, faults, repeatability, load, and exact revisions.

## Fault interpretation

| Fault/status | Required response |
| --- | --- |
| Driver communication / absent | Keep axis disabled; verify UART topology, channel, address straps, common ground, and logic level. |
| Reset detected / undervoltage | Stop; inspect VM and logic supply sequencing, wiring, bulk capacitance, and transients. Rehome after correction. |
| Overtemperature warning | Stop the test soon; reduce current/load or improve cooling and measure temperature. |
| Overtemperature shutdown | Driver is disabled; remove motor power, allow cooling, diagnose before reset, and rehome. |
| Short to ground/supply | Remove power immediately and inspect motor cable, connector, phase pairing, and carrier. |
| Open load | With power removed, inspect continuity/connector; remember the flag may be ambiguous at standstill/low current. |
| Homing stuck / never / release / timeout | Remove motor power if travel is unsafe; inspect switch polarity, mechanics, wiring, direction, debounce, speed, and travel. |
| Unexpected home switch | Stop and rehome only after checking limits, direction, switch noise, and mechanical position. |
| Host heartbeat timeout | Drivers have been stopped/disabled; inspect USB/host failure and rehome because position is untrusted. |
| Emergency stop | Use the physical disconnect as necessary; clear the hazard, release the input, explicitly reset, and rehome. |

## Known limitations and required confirmations

- Exact ESP32 board and TMC2209 carrier revisions/pinouts are unconfirmed.
- Carrier UART one-wire circuitry, address straps, sense resistance, enable polarity,
  and onboard potentiometer interaction are unconfirmed.
- Motor rated current, phase wiring, torque, inductance, thermal limits, and load are
  unconfirmed.
- No encoder, stall verification, closed-loop positioning, backlash compensation, or
  continuous-rotation/slip-ring support exists.
- The 2 µs software STEP timing compiles for ESP32 but has not been measured on a
  scope or logic analyzer under serial/dual-axis load.
- GPIO electrical behavior, e-stop circuit, switch noise, USB backfeed, power
  transient behavior, thermal performance, homing repeatability, cable clearance, and
  coordinated physical motion all remain unvalidated.
- ESP32 compilation, native simulation, and unit tests are not physical validation.
