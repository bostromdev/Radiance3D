# Assembly, routing, and commissioning — Revision 1

## Assembly order

1. Approve the Fusion component-envelope and swept-volume study inside 220 × 220 mm.
2. Print and fit-test the 21.97 mm motor-pilot and 5.00/4.60 mm D-shaft clamp coupons.
3. Print the open base, centred pan-motor mount, direct-drive pan hub/platform, tilt
   mount/hub, AD8317 carriage, reversible board trays, and cable guides.
4. Install both motors without forcing hubs axially against motor faces. Confirm clamp
   fasteners remain accessible and both motors can be replaced.
5. Install the ESP32, two TMC2209s, and two ZX-052s in open reversible retention.
   Preserve USB-C, OLED/buttons, displays, adjustments, pushbuttons, terminals, cooling,
   and wire insertion directions.
6. Install the selected AD8317 vertically in its adjustable edge/strap carriage. Keep
   both SMAs and electrical pads accessible; do not use the PCB as the structural hub.
7. Add the accessible, strain-relieved side/rear `+12V IN` / `GND IN` entry and internal
   distribution. Do not allocate a battery.
8. Route sacrificial silicone wire through neutral pan position and the tilt range.
   Adjust service loop, strain relief, guides, and limits before cutting final harnesses.
9. Cut, label, continuity-test, and strain-relieve final PWR/SIG/MTR harnesses.
10. Set both buck outputs independently to 5.0 V before attaching loads.
11. Commission at low acceleration and inspect hub slip, shaft loading, platform balance,
    collision clearance, cable twist, base stability, driver cooling, and analog noise.

## Moving harness

The moving silicone bundle contains only four tilt-motor phases, AD8317 +5 V/GND,
VOUT, and analog return. It starts at a neutral position, uses strain relief at both
ends, and remains protected from the platform, hubs, motor connectors, and printed
edges. Final lengths follow the CAD motion study.

**Unlimited continuous pan rotation is not allowed.** Target one managed 360° turn and
return toward cable neutral. The future firmware limits and
**Firmware-defined return-to-home strategy to prevent cumulative cable twist** follow
CAD and prototype testing. Tilt targets the largest collision-free range, desirably
near −90° to +90°.

## Commissioning checklist

- [ ] All hardware and swept envelopes remain within the approved 220 × 220 mm base.
- [ ] Direct-drive hubs grip the D-flat, remain removable, and apply no hard axial preload.
- [ ] Pan/tilt cantilevers and CG offsets are recorded; acceleration starts low.
- [ ] No raw 12 V reaches ESP32, AD8317, or GPIO.
- [ ] Buck A and Buck B each meter 5.0 V; their VOUT+ terminals are not connected.
- [ ] Both buck displays, adjusters, buttons, terminals, and tall components are accessible.
- [ ] Every harness is labelled at both ends and matches the wiring standard.
- [ ] Motor phase mapping is correct and no motor is connected while powered.
- [ ] AUT threads directly onto the vertical AD8317 SMA with no RG316 jumper.
- [ ] The moving bundle completes one managed turn without binding, rubbing, or snagging.
- [ ] Tilt collision-free travel is recorded rather than assumed.
- [ ] AD8317 raw ADC/voltage is stored; no dBm is reported without calibration.

External bearings, separate shafts, couplers, mandatory limit switches, and guards are
future options only if prototype tests justify them.
