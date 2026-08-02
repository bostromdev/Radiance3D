# Fusion 360 design spec — precision pass

The enclosure has not yet been designed. All component locations are conceptual only;
the [reference architecture](../docs/hardware/reference-architecture.md) is the official
non-dimensioned placement baseline for Fusion work.

This document is a tighter handoff for the first Radiance3D pan-and-tilt prototype. It turns the broad CAD brief into a concrete implementation plan that is easier to execute in Fusion 360 and easier to review.

## 1. Target architecture

The first prototype should use a two-axis, two-bearing mechanism with the motor shafts protected from side load:

- Pan axis: a rotating platform carried on bearings in the base, driven by the fixed
  stationary-base pan motor through a coupler; target capability is one controlled 360° turn.
- Tilt axis: a tilt support frame carried on bearings and driven by the tilt motor through a coupler.
- The motor shaft should never be the primary radial support for the axis. The bearings carry the load; the motor supplies torque only.
- The antenna active centre should be placed as close as practical to the pan/tilt axis intersection. The target is $0\,\text{mm}$ offset. Any remaining offset must be recorded as a design parameter rather than hidden.
- The cable path must stay clear through full travel. The selected coax must stay above the minimum bend radius for the cable being used.

## 2. Design rules to preserve

- Use millimetres throughout.
- Keep every dimension driven by a user parameter. No hard-coded values in sketches or features.
- Never edit a measured value to fix a fit. Use clearance parameters instead.
- Preserve the measured-vs-design distinction already documented in the repository.
- Design for PETG. Favor simple geometry, ribs, and fillets over thick solid walls.
- Print shallow register features face-up where practical. Include a chamfer at the mouth of every bore and register.
- For every removable fastener, use a proper boss and heat-set insert strategy rather than relying on a thread in plastic.

## 3. Recommended build order in Fusion

Work in this order and stop after each step for review:

1. Pan base
2. Pan motor mount
3. Pan platform
4. Tilt support frame
5. Tilt motor mount
6. Tilt bearing support
7. Antenna cradle
8. Antenna mount adjustment mechanism
9. Cable routing and strain relief
10. Limit switch mounts
11. Electronics tray
12. Counterweight mount (optional)

Do not start the next component until the current one is reviewed.

## 4. Explicit modeling decisions to make

### 4.1 Motor orientation

Each motor needs an explicit orientation decision, not an assumption.

For each motor, the model must show:
- shaft direction (up/down)
- the location of the rear connector envelope
- the 21.4 mm connector-clearance volume behind the motor, including the 12 mm bend-clearance allowance

The model should state whether the connector envelope is absorbed into the base, a side wall, or a rear wall. The orientation choice must be justified in the design notes.

### 4.2 Bearing layout

Each axis should be supported by two bearings with a defined spacing. The bearing seats must:
- be modelled from the actual bearing dimensions once available
- use a press-fit strategy for the bearing outer race
- not rely on the motor shaft for radial support

### 4.3 Axis intersection and antenna placement

The antenna active centre should be brought as close as practical to the pan/tilt axis intersection. If that cannot be achieved, the model must carry a non-zero offset parameter and the reason should be documented.

### 4.4 Cable path

The cable route should be planned before the frame is finalized. RF and electrical
cabling must be modelled as separate routes: RG316 50 Ω coax is represented with centre
conductor and shield for RF-### paths, while PWR-###, SIG-###, and MTR-### are silicone
wire harnesses. Cable routing must account for:
- bend radius of the moving silicone harness; the external VTX RG316 path is not a
  scanner-mounted antenna-to-detector jumper
- connector reach
- axis travel
- strain relief
- clearance around the moving platform and frame

## 5. Part-number placeholders

You already have the relevant hardware in the project, so part numbers are optional for this first mechanical pass. If you want a cleaner BOM later, record them as placeholders in the model notes and in the BOM. Do not guess.

| Item | Placeholder field | Notes |
|---|---|---|
| Pan bearing | pPanBearingPartNumber | Fill once the bearing is selected |
| Tilt bearing | pTiltBearingPartNumber | Fill once the bearing is selected |
| Pan coupler | pPanCouplerPartNumber | Fill once the coupler is selected |
| Tilt coupler | pTiltCouplerPartNumber | Fill once the coupler is selected |
| Heat-set insert | pHeatSetInsertPartNumber | Fill once the insert supplier is known |
| Limit switch | pLimitSwitchPartNumber | Fill once the switch is selected |
| Antenna mount / connector | pAntennaMountPartNumber | Fill once the antenna hardware is selected |
| Fasteners | pFastenerPartNumber | Fill once the screw sizes are confirmed |

## 6. Measurements still required before final fit

The following measurements should be captured before the design is considered complete for print:

- NEMA 17 motor face width across flats
- NEMA 17 mounting-hole spacing and diameter
- NEMA 17 shaft diameter
- Bearing inner diameter, outer diameter, and width
- Coupler outer diameter, length, and bores
- Heat-set insert outer diameter, length, and pilot diameter
- Limit switch body and mounting-hole dimensions
- Antenna body, connector, and coax dimensions
- Fastener sizes and lengths

If any of these are not available yet, leave the corresponding features parametric and mark them as provisional in the model.

## 7. Deliverables for each component

For each component in the build order, the Fusion model should include:
- the geometry for the part
- its mounting interfaces
- its clearances for adjacent parts
- the print orientation note
- the fastener or insert strategy
- any provisional dimensions that still need measurement

## 8. Suggested review checklist

Before moving on from any component, confirm:
- the part is fully parameterized
- the fit is based on the measured dimension, not a guessed one
- the print orientation is explicit
- the fastener strategy is explicit
- the connector and cable-clearance envelopes are respected
- any unresolved dimension is clearly marked as provisional
