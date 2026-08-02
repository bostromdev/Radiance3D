# Fusion 360 Assistant prompt

Paste everything inside the code block below into Fusion 360 Assistant. It is written
to be self-contained — it carries the measured dimensions with it, so the assistant
does not need access to this repository.

Source of the numbers: [`nema17.md`](nema17.md), [`tmc2209-v1.3.md`](tmc2209-v1.3.md),
[`esp32-devkit.md`](esp32-devkit.md), [`fusion360-parameters.md`](fusion360-parameters.md),
[`missing-measurements.md`](missing-measurements.md).

---

````text
You are helping me design the first Radiance3D pan-and-tilt prototype in Fusion 360.

Radiance3D is an antenna radiation-pattern measurement platform. The mechanism rotates
an antenna under test (AUT) in azimuth (pan) and elevation (tilt) while a stationary
receiver measures it. Positioning repeatability matters more than speed, and the
structure must not distort the radiation pattern more than necessary.

=====================================================================
NON-NEGOTIABLE RULES
=====================================================================

1. Millimetres throughout. Never switch units.
2. Every dimension must come from a named user parameter. No hard-coded numbers in
   any sketch or feature.
3. Each major part is a separate Fusion component, named as listed below.
4. The MEASURED dimensions below are the source of truth. They were taken with
   calipers on the actual parts. Do not replace them with datasheet or "standard"
   values, even where a standard value looks rounder or more familiar.
5. Never guess a critical fit dimension. Anything marked PROVISIONAL is a placeholder
   for a measurement I have not taken yet. If a feature's fit depends on a PROVISIONAL
   value, say so out loud before modelling it, and design that feature so the
   parameter can change without rebuilding the part.
6. Design for PETG on a fused-filament printer:
   - avoid overhangs steeper than 45 degrees where a design change can prevent them
   - minimise support material; state the intended print orientation for each part
   - use structural fillets at load-bearing intersections and ribs instead of thick
     solid walls
   - use heat-set threaded inserts wherever a fastener will be removed more than once
7. Protect the motor shafts. A motor shaft must not carry a side load or a cantilevered
   mass. Every rotating axis is carried on its own bearings; the motor supplies torque
   through a coupler only.
8. Place the pan and tilt axes as close together as practical, and put the antenna's
   active centre as close as practical to the point where the two axes intersect.
   Offset between the antenna phase centre and the axis intersection is a measurement
   error, not just an aesthetic issue. Report any offset you cannot eliminate.
9. Route cables so they never bind, stretch or wrap as the axes move. Coax bend radius
   is a hard constraint, not a guideline.
10. Work on ONE major component at a time. Do not start the next component until I
    have reviewed and approved the current one.

=====================================================================
MEASURED DIMENSIONS - SOURCE OF TRUTH
=====================================================================

NEMA 17 stepper motor, YEJMKJ / LYLANMO, quantity 2 (one pan, one tilt):
  Motor body diagonal, corner to corner       54.30 mm
  Motor body length, face plate to rear face  20.76 mm   (short-body / "pancake")
  Rear connector protrusion above end cap      9.38 mm
  Rear connector housing length               16.43 mm   (6-position housing)

BIGTREETECH TMC2209 V1.3 driver, quantity 2, measured WITH heatsink installed:
  PCB length                                  20.14 mm
  PCB width                                   15.14 mm
  Heatsink left edge to PCB right edge        12.21 mm
  Heatsink right edge to PCB left edge        11.22 mm
  Heatsink front edge to PCB back edge        13.73 mm
  Overall installed height,
    heatsink top to header pin tips           22.24 mm
  Note: the heatsink is NOT centred on the PCB. The left and right offsets differ by
  about 1 mm, so the driver is handed. Do not model a symmetric pocket.

ELEGOO ESP32 devkit with ESP-WROOM-32, USB-C, quantity 1:
  PCB length                                  51.47 mm
  PCB width                                   28.23 mm
  PCB thickness                                1.33 mm   (NOT 1.6 mm)
  Board height including module                4.38 mm   (excludes header pins below)
  USB-C receptacle width                       8.83 mm   (plug overmould is wider)

=====================================================================
PROVISIONAL VALUES - NOT MEASURED, DO NOT TRUST
=====================================================================

I have not measured these. The values are nominal placeholders so the model can be
built and updated later. Flag every feature that depends on one.

  Motor face width across flats               42.30 mm   (photo shows "42", decimals unreadable)
  Motor mounting-hole spacing                 31.00 mm
  Motor mounting-hole diameter                 3.40 mm
  Motor pilot boss diameter                   22.00 mm
  Motor pilot boss height                      2.00 mm
  Motor shaft diameter                         5.00 mm   (photo reads "4.9?")
  Motor shaft length                          20.00 mm
  Bearing inner diameter                       8.00 mm
  Bearing outer diameter                      22.00 mm
  Bearing width                                7.00 mm
  Coupler outer diameter                      19.00 mm
  Coupler length                              25.00 mm
  Coupler bore, motor side                     5.00 mm
  Coupler bore, driven side                    8.00 mm
  Heat-set insert outer diameter               4.60 mm
  Heat-set insert length                       5.70 mm
  Heat-set insert pilot diameter               4.00 mm
  Limit switch body length                    20.00 mm
  Limit switch mounting-hole spacing           9.50 mm
  Antenna body diameter                       10.00 mm
  Coax outer diameter                          3.00 mm
  Coax minimum bend radius                    15.00 mm

=====================================================================
DESIGN CHOICES - MINE, ADJUSTABLE
=====================================================================

  Wall thickness                               3.00 mm
  Base thickness                               6.00 mm
  General print clearance                      0.30 mm
  Bearing press-fit allowance                  0.05 mm
  Free air above the driver heatsink           8.00 mm
  Cable bend clearance behind a connector     12.00 mm
  Mounting clearance                           1.00 mm
  Structural fillet radius                     3.00 mm
  Cosmetic fillet radius                       1.00 mm
  Rib thickness                                2.40 mm
  Tilt axis height above the pan platform     90.00 mm

=====================================================================
COMPONENTS TO DESIGN, IN ORDER
=====================================================================

  1.  Pan_Base                  Stationary base. Carries everything.
  2.  Pan_Motor_Mount           Holds the pan NEMA 17. Shaft down or up, your
                                recommendation with reasoning.
  3.  Pan_Platform              Rotating platform, carried on a bearing, not on the
                                motor shaft.
  4.  Tilt_Support_Frame        Rises from the pan platform. Carries the tilt axis at
                                both ends.
  5.  Tilt_Motor_Mount          Holds the tilt NEMA 17 on one side of the frame.
  6.  Tilt_Bearing_Support      Opposite side of the frame. Second bearing for the
                                tilt axis so the motor shaft carries no side load.
  7.  Antenna_Cradle            Rotates with the tilt axis. Holds the AUT.
  8.  Antenna_Mount_Adjustable  Lets the antenna's active centre be positioned onto
                                the axis intersection.
  9.  Cable_Routing             Guides, clips and strain relief through the moving
                                envelope.
  10. Limit_Switch_Mounts       One adjustable homing switch per axis.
  11. Electronics_Tray          Removable. Holds the ESP32, both TMC2209 drivers and
                                the buck converter.
  12. Counterweight_Mount       Optional. Balances the tilt axis about its pivot.

=====================================================================
WHAT TO DO FIRST - DO NOT SKIP TO MODELLING
=====================================================================

Before you create any geometry, do all seven of these and stop for my approval:

  1. Review the measured dimensions above.
  2. List every measurement you understand and what you will use it for.
  3. List every dimension that is missing or ambiguous and that you need from me
     before the design can be finished. Be specific about which feature is blocked.
  4. Create the initial Fusion user-parameter table, grouped as Measured, Provisional,
     Design choice and Calculated. Put the word PROVISIONAL in the comment field of
     every provisional parameter.
  5. Explain your proposed pan-and-tilt architecture: bearing arrangement, how each
     motor shaft is protected from side loading, how close the two axes come to
     intersecting, where the antenna's active centre sits relative to that
     intersection, and how the cable route stays clear through the full travel.
  6. Design fit-test coupons FIRST for every uncertain dimension — the bearing seat,
     the motor pilot boss register, the motor mounting-hole pattern, the shaft
     coupler bore and the heat-set insert boss. These are small printable test pieces,
     not full parts. Tell me what to print and what to measure on each one.
  7. Only after I have reviewed that measurement review, begin Pan_Base. Model
     Pan_Base only. Stop when it is complete and wait for my approval before starting
     Pan_Motor_Mount.

Do not model all twelve components in one pass. Do not substitute a standard value for
a measured one. If something is unclear, ask me instead of assuming.
````

---

## After the first component

When Fusion Assistant reports back:

- Check that no feature silently used a PROVISIONAL value where a real measurement
  exists.
- Print the fit-test coupons before approving `Pan_Base`. A bearing seat or a motor
  register that is wrong by 0.2 mm will not be visible on screen.
- Feed any newly taken measurements back into the component file in this folder first,
  then update [`fusion360-parameters.md`](fusion360-parameters.md), then update the
  Fusion parameter. The measurement file stays canonical.
