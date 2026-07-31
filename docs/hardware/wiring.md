# Wiring

Verified pin assignments do not exist. The Version 1 wiring record must identify the
exact ESP32 board and every connector pin, signal reference, voltage domain, wire
rating, shielding, grounding point, switch behavior, and emergency isolation before a
physical build is called supported.

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

Disconnect external and USB power before changing wiring. Because USB and a buck
converter may energize the board simultaneously, explicitly document the chosen
backfeed prevention or supported dual-power behavior. Verify continuity, polarity,
grounding, and absence of shorts before energizing motor power.
