# Autodesk Fusion handoff — Radiance3D Revision 1

Paste the following into Autodesk Fusion or a CAD-design agent.

```text
Design Radiance3D Revision 1: an open PETG desktop instrument that rotates an
interchangeable 5.8 GHz Antenna Under Test for radiation-pattern measurement. Optimize
measurement repeatability, stability, RF consistency, serviceability, then appearance.

Start with a component-envelope layout study. Do not create full structural geometry
first. Solve the smallest stable arrangement inside a hard 220 × 220 mm base boundary,
center the pan axis, determine the smallest practical lightweight pan platform, place
stationary electronics outside its swept envelope, and present dimensions, access
envelopes, and moving sweeps for approval. Use engineering judgment for reversible
prototype choices rather than repeatedly asking for noncritical dimensions.

FIXED ARCHITECTURE
- Open base; electronics remain visible and serviceable. No sealed enclosure or battery.
- Centred stationary pan NEMA-17 directly drives a lightweight platform through a
  removable clamp-style hub on its 5 mm D-shaft.
- The rotating platform carries the tilt NEMA-17, direct-drive tilt hub/carriage,
  vertical AD8317, interchangeable AUT, cable guides, and moving silicone harness only.
- No external bearings, separate shafts, shaft couplers, slip ring, or mandatory limit switches.
- AD8317 EVAL BD / NWDZ V1.0 is selected. The AUT threads directly onto its SMA; no
  AUT-to-detector coax. The external VTX stays stationary and off-scanner.
- Stationary base carries ESP32, two TMC2209 V1.3 drivers, two ZX-052 V2.0 converters,
  external 12 V/GND entry, distribution, and open wiring.
- Power: direct 12 V to both driver VM/GND branches; Buck A gives regulated 5.0 V to
  ESP32; Buck B independently gives regulated 5.0 V to AD8317. Never parallel outputs;
  do not add a third converter.

MEASURED ENVELOPES, MM
NEMA-17: 43.46 maximum face/body clearance envelope; 20.84 safe body length;
21.97 × 2.00 pilot; 31.00 × 31.00 nominal-square hole pattern; 5.00 shaft major;
4.60 flat-to-opposite thickness; 0.20 derived radial flat depth; 22.39 shaft length
from pilot face; rear connector 9.53 protrusion and 16.44 housing envelope.
TMC2209 each: 20.14 × 15.14 PCB; 22.24 installed height.
ESP32: 51.47 × 28.23 PCB; 1.33 PCB thickness; 4.38 top-side height; 8.83 USB-C shell.
ZX-052 each: 66.07 × 36.48 PCB. VIN is at one end, VOUT at the other; display,
adjuster and pushbutton face upward. The approximately 2.15 edge reading is not total height.
AD8317: 36.27 × 35.72 PCB; 55.84 SMA-tip-to-tip; 9.78 protrusion per side;
17.36 shield-can length. Approximately 40.37 diagonal feature readings are not a hole pattern.

DIRECT DRIVE
Use removable D-profile split clamps, captive-nut clamps, or equivalent compact hubs.
Do not use a loose round PETG bore, a separate coupler, a set screw cutting into PETG,
or hard axial preload. Keep rotating mass and shaft cantilevers small, balance the tilt
load, keep fasteners accessible, and make both motors replaceable. The AD8317 PCB is
not the structural hub. Use vertical edge/strap retention plus a lightweight preferably
nonconductive guide to relieve antenna bending load without blocking SMA access.

OPEN ELECTRONICS
Use adjustable edge clips, shallow trays, rails, straps, removable crossbars, or zip-tie
slots; missing PCB hole coordinates do not block CAD. Preserve ESP32 USB-C/buttons/OLED,
TMC2209 heatsink/VREF/header/wiring access, both buck displays/adjusters/pushbuttons/
terminals, screwdriver and wire-insertion directions, tall-component clearance,
ventilation, and 18/22 AWG wire bends. Separate motor wiring from the AD8317 analog route.

MOTION AND HARNESS
Pan target is one managed complete 360° turn from a neutral cable position, then return
toward neutral; never unlimited winding. Tilt target is the largest collision-free range,
desirably near −90° to +90°, with actual travel reported from CAD. Model one flexible
bundle containing four tilt phases, AD8317 +5 V, GND, VOUT, and analog return. Include
service loop, strain relief at both ends, bend clearance, guides, and protection from
the platform, hubs, and motor connectors. Exact wire lengths follow CAD.

CAD RULES
Use mm and named parameters: m=measured/frozen, c=calculated, d=design choice,
p=provisional. Never alter m-values to tune print fit. Design PETG parts with ribs,
structural fillets, minimal support, explicit print orientation, removable modules,
open wiring, and editable M3-compatible provisional hardware. Print motor-pilot and
5.00/4.60 D-shaft clamp coupons before full hubs or platforms. Do not invent purchased parts.

Before structural geometry, report the proposed base and platform dimensions, component
arrangement, connector/service envelopes, swept volumes, pan/tilt cantilever distances,
estimated moving mass when possible, both shaft CG offsets, achievable tilt range,
one-turn harness behavior, and any direct-drive loading concern. Prove every component
remains within 220 × 220 mm. Then design one subsystem at a time and pause for approval.
Keep every choice reversible and parameterized.
```
