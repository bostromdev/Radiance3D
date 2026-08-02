# Fusion 360 build order — first prototype

The enclosure has not yet been designed. All locations in this document are conceptual
placement intent only; use the [reference architecture](../docs/hardware/reference-architecture.md)
as the official non-dimensioned baseline before choosing CAD dimensions.

This is the practical next-step plan for the first Radiance3D prototype. It is intentionally scoped so you can build the structure one component at a time without guessing.

## 0. Before modeling anything

Before you model the first part, confirm these items from the existing measurements:

- Motor body length and pilot boss dimensions are known.
- Motor shaft length from the boss face is known.
- The rear connector envelope is known and must be preserved.
- The clearances for PETG are set as design parameters, not hard-coded.
- Any missing motor-mount dimensions are marked provisional and not silently assumed.

## 1. Build Pan_Base first

Create the stationary base first. This defines the envelope for everything else.

The 12 V source is permanently off-board beneath the workbench. Do not model a battery
compartment, retention feature, or battery-weight allowance. Model only an accessible,
strain-relieved `+12V IN` / `GND IN` entry and its route to internal distribution.

### Goals
- Provide a stable platform for the pan axis.
- Reserve space for the pan motor, bearings, and the electronics tray.
- Leave enough clearance for the rear motor connector and cable routing.
- Keep the pan/tilt axis intersection as close as practical to the antenna centre.

### Model requirements
- Use a base plate and side walls or ribs to create a stiff structure.
- Include a mounting pocket or bracket for the pan motor.
- Add a bearing seat for the pan axis.
- Leave a defined cable path from the pan motor and the tilt assembly toward the electronics area.
- Keep the base thick enough to resist flexing under the tilt mass.

### Review questions
- Does the pan axis sit at a sensible height for the antenna?
- Does the motor connector have clear access without blocking the axis?
- Is the base wide enough to avoid wobble from the tilt assembly?

## 2. Build Pan_Motor_Mount second

Once the base is reviewed, add the pan motor mount.

### Goals
- Hold the pan motor so its shaft is aligned with the pan axis.
- Keep the motor shaft protected from side load.
- Allow the motor to be installed and serviced without breaking the structure.

### Model requirements
- Use the measured motor pilot diameter and height.
- Create a printed register or pocket sized from the measured values plus clearance.
- Add a mounting-hole pattern only once the actual motor hole spacing and diameter are measured.
- If the motor is mounted shaft-up or shaft-down, document that decision and the reason.
- Preserve the rear connector clearance volume.

### Review questions
- Is the motor face parallel to the pan axis?
- Is its shaft aligned with the pan bearing centreline?
- Is the connector still reachable?

## 3. Build Pan_Platform third

Now model the rotating platform that carries only the tilt motor, vertically mounted
AD8317 detector, and antenna mount. The AUT threads directly onto the detector SMA;
do not add an antenna-to-detector coax jumper.

### Goals
- Create the pan platform as a bearing-supported rotating part.
- Keep the platform stiff enough that the tilt assembly does not make it flex.
- Make the platform large enough for the tilt support structure and cable routing.

### Model requirements
- Add a bearing seat for the pan bearing.
- Provide a clear interface to the tilt support frame.
- Keep the antenna centre close to the axis intersection.
- Reserve space for the tilt motor and support structure.

### Review questions
- Does the platform leave enough area for the tilt frame without clipping the base?
- Does the platform provide a stable interface for the next component?

## 4. Build Tilt_Support_Frame fourth

This is the vertical structure that raises the tilt axis above the pan platform.

### Goals
- Raise the tilt axis to a useful antenna height.
- Keep the frame rigid and not overly tall.
- Support both ends of the tilt shaft so the motor does not carry side load.

### Model requirements
- Create a frame that rises from the pan platform.
- Include two bearing support interfaces, one on each side of the tilt axis.
- Keep the tilt axis close to the pan axis intersection.
- Leave cable routing room through the frame.

### Review questions
- Does the frame create an obvious, stiff load path?
- Is the tilt axis height reasonable for the antenna and receiver geometry?
- Is there enough room for a coupler and a motor mount without collision?

## 5. Build Tilt_Motor_Mount fifth

Add the tilt motor mount on one side of the frame.

### Goals
- Hold the tilt motor in a way that does not twist the support frame.
- Align the motor shaft with the tilt axis.
- Keep the motor out of the path of the antenna and the cable route.

### Model requirements
- Use the measured motor register and pilot dimensions.
- Model the motor mount from the measured hardware, not assumptions.
- Leave space for the motor connector and cable bend.

### Review questions
- Is the motor mount sufficiently stiff?
- Does it create a clear torque path into the coupler and bearing-supported shaft?

## 6. Build Tilt_Bearing_Support sixth

Add the opposite support for the tilt axis.

### Goals
- Complete the two-bearing support for the tilt axis.
- Ensure the tilt shaft is not cantilevered from the motor.
- Provide the second bearing seat needed for repeatable motion.

### Model requirements
- Create a bearing support that matches the frame geometry.
- Ensure the tilt axis runs through the intended centreline.
- Keep the frame symmetrical enough to avoid skew.

## 7. Build Antenna_Cradle seventh

Now create the part that holds the antenna or DUT.

### Goals
- Mount the antenna at the correct point in space.
- Put the antenna active centre as close as possible to the pan/tilt axis intersection.
- Keep the antenna physically stable and mechanically isolated from the frame.

### Model requirements
- Include a place to attach the antenna body and connector.
- Leave room for the coax to route without tight bends.
- Keep the mounting feature adjustable if needed.

### Review questions
- Is the antenna centre aligned with the intended measurement point?
- Does the fixture create unnecessary RF disturbance?
- Can the antenna be installed and removed easily?

## 8. Build Antenna_Mount_Adjustable eighth

Add the adjustable mount if the antenna cradle needs positional tuning.

### Goals
- Make the antenna centre adjustable in a controlled way.
- Support calibration and alignment without reprinting the whole cradle.

### Model requirements
- Keep the adjustment mechanism simple and printable.
- Avoid a design that depends on fragile thin features.
- Make the adjustment range explicit in the model.

## 9. Build Cable_Routing ninth

Cable routing should be designed as part of the structure, not added later.

### Goals
- Prevent cable binding or stretching through motion.
- Model RF paths as RG316 50 Ω coax with a centre-and-shield envelope; model silicone
  electrical harnesses separately, never as the same cable type.
- Respect the measured minimum bend radius of the RG316 coax and the bend clearance of
  each silicone harness.
- Keep the cable path away from fasteners and the moving bearing faces.

### Model requirements
- Include clips or guides in the frame and base.
- Leave generous bend radius room.
- Prevent either cabling system from wrapping around the axis as it moves.
- Label CAD routes with `RF-###` for coax and `PWR-###`, `SIG-###`, or `MTR-###` for
  silicone harnesses.

## 10. Build Limit_Switch_Mounts tenth

Add the homing or limit switches once the structure is stable.

### Goals
- Provide repeatable homing so the position is not lost on reset.
- Mount the switches without relying on fragile printed geometry alone.

### Model requirements
- Keep switch mounts adjustable if needed.
- Use the measured switch dimensions once available.
- Avoid overconstraining the moving structure with the switch mount.

## 11. Build Electronics_Tray eleventh

Once the mechanics are settled, add the electronics tray.

### Goals
- Hold the ESP32, both TMC2209 drivers, and the buck converter.
- Keep wiring short and serviceable.
- Preserve access to connectors and USB.

### Model requirements
- Use the measured board dimensions.
- Include pockets or slots for boards with clearance.
- Reserve room for the USB cable and wiring harness.

## 12. Optional Counterweight_Mount last

Only if the tilt assembly needs it.

### Goals
- Balance the tilt axis and reduce motor torque demand.
- Improve repeatability under acceleration.

### Model requirements
- Make the counterweight mount adjustable.
- Keep it nonintrusive to the antenna path.
