# Fusion 360 design spec — Revision 1 direct-drive prototype

## Scope

Design an open PETG desktop scanner inside a 220 × 220 mm maximum base. Center the
stationary pan motor, solve the smallest stable direct-drive platform, and arrange the
open stationary electronics outside its swept envelope. Do not prescribe coordinates
or platform diameter before the layout study.

## Fixed architecture

- Pan motor is stationary and directly drives a lightweight platform through a
  removable clamp-style hub on its measured 5 mm D-shaft.
- Tilt motor rotates with the platform and directly drives a lightweight detector/AUT
  carriage through the same kind of removable hub.
- Motor internal bearings support Revision-1 loads. No external bearings, separate
  shafts, couplers, or slip ring are used.
- The selected AD8317 is vertical. Interchangeable AUTs thread directly onto its SMA;
  there is no detector-to-AUT coax.
- ESP32, both TMC2209s, both ZX-052 boards, 12 V entry, and distribution are stationary.
- The open base has no electronics enclosure or battery compartment.

## Layout study

Before printable geometry, Fusion must:

1. Model measured component envelopes in their access orientations.
2. Center the pan axis and determine the smallest practical pan platform.
3. Place stationary boards outside the platform sweep and inside 220 × 220 mm.
4. Show connector, terminal, display, adjuster, cooling, wire-bend, and service envelopes.
5. Show tilt and interchangeable-AUT collision envelopes.
6. Show the moving harness at neutral and both ends of one managed pan turn.
7. Report platform size, base size, cantilever distances, estimated moving mass when
   known, both shaft CG offsets, achievable tilt range, and direct-drive concerns.

## Direct-drive hubs and carriage

Use a D-profile split clamp, captive-nut clamp, or equivalent removable clamping hub.
Do not use a loose round PETG bore, a separate shaft coupler, a set screw cutting into
PETG, or hard axial preload against a motor. Keep the carriage close to the shaft and
all clamp fasteners accessible. The AD8317 PCB is retained in a vertical adjustable
edge/strap holder; it is not the hub or primary structural member.

## Open electronics

Use reversible clips, shallow trays, rails, straps, removable crossbars, or zip-tie
slots. Missing PCB hole coordinates do not block the design. Preserve USB-C, ESP32
buttons/OLED, TMC2209 VREF/heatsink/header access, both ZX-052 displays/adjusters/
pushbuttons/terminals, tall-component ventilation, and 18/22 AWG wire bends.

## Motion and harness

Pan targets one managed 360° turn from cable neutral and returns toward neutral; it is
not unlimited rotation. Tilt targets the largest collision-free range, desirably near
−90° to +90°. Route four tilt phases, AD8317 +5 V/GND, VOUT, and analog return as one
flexible silicone bundle with service loop, strain relief, guides, and edge protection.
Keep analog wiring separated from motor VM/phase wiring where practical.

## PETG construction

Use named parameters, ribs instead of unnecessary solid mass, structural fillets,
minimal supports, explicit print orientations, open wire routing, M3-compatible
provisional hardware, and removable modules. Print motor-pilot and D-shaft hub coupons
before the full platform or tilt carriage. Never edit measured values to tune fit.

External bearing-supported axes, limit switches, and guards are future upgrades only.
