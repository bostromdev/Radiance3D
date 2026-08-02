# Wiring

The Version 1 wiring baseline uses an ESP32 development board, two BIGTREETECH TMC2209
V1.3 carriers, 12 V motor power, a regulated 5.0 V logic rail, and configurable home
switches. Final pin assignments remain to be confirmed against the exact ESP32 board
revision and carrier labels. The current mapping is documented in the
[TMC2209 commissioning guide](tmc2209-commissioning.md). The Version 1 wiring record
must identify the exact board and every connector pin, signal reference, voltage
domain, wire rating, shielding, grounding point, switch behavior, and emergency
isolation before a physical build is called supported.

## Motor phase wiring

The motors are model `42HDB0014NC-24B`. The nameplate gives the coil pairing directly:

| Wire colour | Phase |
|---|---|
| Black | A+ |
| Green | A− |
| Red | B+ |
| Blue | B− |

Coil A is **black + green**. Coil B is **red + blue**. Splitting a pair across the two
driver outputs makes the motor buzz, jitter or lock instead of turning, and it is not
always obvious from the sound which pair is wrong.

Confirm with a multimeter before energising: about 3.5 Ω within a pair, open circuit
between pairs. Phase resistance and the rest of the nameplate data are recorded in
[`Measurements/nema17.md`](../../Measurements/nema17.md).

Motors must never be connected or disconnected while the drivers are powered.

## Required electrical boundaries

- ESP32 logic is 3.3 V. Do not assume a driver module, switch module, display, or
  future sensor is 5 V tolerant.
- Each driver receives 12 V motor power, common ground, 3.3 V-compatible STEP/DIR,
  firmware-controlled enable, and a reviewed UART connection.
- Each axis has one independently configured home input. Normally-closed wiring is
  recommended, with pull-up/pull-down choice and debounce documented.
- Emergency stop has a dedicated input and accessible power-isolation behavior. A
  firmware input alone is not the only protection against unexpected motion.
- Bulk motor-supply capacitance and local logic decoupling must appear on the wiring
  record with values and voltage ratings selected from component guidance.

Route motor and power wiring away from STEP/DIR, switch inputs, receiver leads, and RF
feedlines. Cross sensitive and noisy runs at right angles where separation is limited.
Provide strain relief and verify clearance through the entire configured azimuth and
elevation envelope. Consistent coax routing is part of the RF experiment, not an
afterthought.

Recommended wire usage:

- 18 AWG: battery, main power, and power distribution
- 22 AWG: driver power, motor extensions, and buck output
- 26 AWG: STEP, DIR, UART, ENABLE, DIAG, and switch signals

Separate signal wiring from motor wiring and keep high-current loops short.

Disconnect external and USB power before changing wiring. Because USB and a buck
converter may energize the board simultaneously, explicitly document the chosen
backfeed prevention or supported dual-power behavior. Verify continuity, polarity,
grounding, and absence of shorts before energizing motor power.
