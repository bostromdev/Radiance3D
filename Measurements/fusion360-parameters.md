# Fusion 360 parameters

Proposed Fusion 360 user parameters for the Radiance3D pan-and-tilt prototype.

Every parameter is tagged with its **type**, and the types are never mixed silently:

| Type | Meaning |
|---|---|
| **Measured** | Taken from a caliper reading traceable to a photograph in this folder |
| **Design choice** | Chosen by the designer; no physical measurement behind it |
| **Provisional** | A placeholder standing in for a measurement that has not been taken. **Must be replaced before any part is printed for fit.** |
| **Calculated** | Derived from other parameters by expression |

All values are millimetres unless the parameter is an angle.

> [!WARNING]
> Every **Provisional** row is a hole in the design. They are listed in
> [`missing-measurements.md`](missing-measurements.md) with instructions for taking
> the real measurement. Do not treat a provisional value as verified because it looks
> plausible.

---

## Measured

Traceable to a caliper reading and a photograph.

| Parameter | Expression | Source | Type | Notes |
|---|---:|---|---|---|
| `MotorBodyDiagonal` | 54.30 mm | nema17.md — IMG_5223 | Measured | Corner to corner across chamfered corners |
| `MotorBodyLength` | 20.76 mm | nema17.md — IMG_5231 | Measured | Front face plate to rear face plate, excludes pilot boss |
| `MotorConnectorProtrusion` | 9.38 mm | nema17.md — IMG_5226, IMG_5227 | Measured | Height of rear connector above end-cap face; two independent readings agreed |
| `MotorConnectorLength` | 16.43 mm | nema17.md — IMG_5228 | Measured | 6-position housing on the rear end cap |
| `MotorShaftLengthFromBoss` | 22.39 mm | nema17.md — operator measurement | Measured | **Datum is the pilot boss face, not the motor face plate.** No source photograph |
| `DriverPcbLength` | 20.14 mm | tmc2209-v1.3.md — IMG_5234 | Measured | Bare PCB long axis |
| `DriverPcbWidth` | 15.14 mm | tmc2209-v1.3.md — IMG_5233, IMG_5238 | Measured | Bare PCB short axis; repeat read 15.16 mm |
| `DriverHeatsinkLeftToPcbRight` | 12.21 mm | tmc2209-v1.3.md — IMG_5235 | Measured | Heatsink left face to far PCB edge |
| `DriverHeatsinkRightToPcbLeft` | 11.22 mm | tmc2209-v1.3.md — IMG_5236 | Measured | Heatsink right face to far PCB edge |
| `DriverHeatsinkFrontToPcbBack` | 13.73 mm | tmc2209-v1.3.md — IMG_5237 | Measured | Heatsink front face to far PCB edge |
| `DriverInstalledHeight` | 22.24 mm | tmc2209-v1.3.md — IMG_5239 | Measured | Heatsink top to header-pin tips; full envelope |
| `Esp32PcbLength` | 51.47 mm | esp32-devkit.md — IMG_5241 | Measured | Bare board outline |
| `Esp32PcbWidth` | 28.23 mm | esp32-devkit.md — IMG_5242 | Measured | Bare board outline |
| `Esp32PcbThickness` | 1.33 mm | esp32-devkit.md — IMG_5245 | Measured | Thinner than the usual 1.6 mm assumption |
| `Esp32BoardHeightWithModule` | 4.38 mm | esp32-devkit.md — IMG_5246 | Measured | PCB plus ESP-WROOM-32; excludes header pins below |
| `Esp32UsbCWidth` | 8.83 mm | esp32-devkit.md — IMG_5243, IMG_5244 | Measured | Receptacle shell only; plug overmould is wider |

## Calculated

Derived by expression. Fusion will keep these correct when their inputs change.

| Parameter | Expression | Source | Type | Notes |
|---|---:|---|---|---|
| `MotorPocketDepth` | `MotorBodyLength + PrintClearance` | Calculated | Calculated | Pocket for the motor body only |
| `MotorRearClearance` | `MotorConnectorProtrusion + CableBendClearance` | Calculated | Calculated | Space behind the motor for the connector and its plug. 21.38 mm at current values — this decides whether the pan motor can face shaft-up |
| `DriverBayHeight` | `DriverInstalledHeight + HeatsinkAirGap` | Calculated | Calculated | Never make the bay exactly `DriverInstalledHeight`; the heatsink needs air |
| `DriverHeatsinkWidth` | `DriverHeatsinkLeftToPcbRight + DriverHeatsinkRightToPcbLeft - DriverPcbWidth` | Calculated | Calculated | 8.29 mm. Fully measured — no provisional input |
| `DriverHeatsinkOffset` | `(DriverHeatsinkLeftToPcbRight - DriverHeatsinkRightToPcbLeft) / 2` | Calculated | Calculated | 0.495 mm from the PCB centre. This is why the driver is handed |
| `DriverHeatsinkFrontEdge` | `DriverPcbLength - DriverHeatsinkFrontToPcbBack` | Calculated | Calculated | 6.41 mm from the PCB front edge. The heatsink's **back** edge is unknown — see `missing-measurements.md` |
| `Esp32PocketLength` | `Esp32PcbLength + 2 * PrintClearance` | Calculated | Calculated | Tray pocket, removable fit |
| `Esp32PocketWidth` | `Esp32PcbWidth + 2 * PrintClearance` | Calculated | Calculated | Tray pocket, removable fit |
| `MotorFaceHalf` | `MotorFaceWidth / 2` | Calculated | Calculated | Used to centre the motor on its mount |
| `MotorShaftLengthFromFace` | `MotorShaftLengthFromBoss + MotorPilotHeight` | Calculated | Calculated | The vendor-style figure. **Depends on the provisional `MotorPilotHeight`** — prefer the boss datum, which is measured |
| `CouplerShaftEngagement` | `CouplerLength / 2` | Calculated | Calculated | Motor-side engagement. Must stay below `MotorShaftLengthFromBoss` (22.39 mm) |
| `BearingSeatBore` | `BearingOuterDiameter - BearingFitAllowance` | Calculated | Calculated | Press-fit seat; validate with a coupon first |

## Provisional — must be replaced before printing for fit

These are placeholders. Each one has an entry in
[`missing-measurements.md`](missing-measurements.md).

| Parameter | Expression | Source | Type | Notes |
|---|---:|---|---|---|
| `MotorFaceWidth` | 42.30 mm | **Provisional** | Provisional | IMG_5221 reads `42` but decimals are unreadable. 42.30 is the NEMA 17 nominal, not a measurement |
| `MotorHoleSpacing` | 31.00 mm | **Provisional** | Provisional | NEMA 17 nominal square pattern. Not verified on these motors |
| `MotorHoleDiameter` | 3.40 mm | **Provisional** | Provisional | M3 clearance, assumed. Not measured |
| `MotorPilotDiameter` | 22.00 mm | **Provisional** | Provisional | NEMA 17 nominal. Not measured |
| `MotorPilotHeight` | 2.00 mm | **Provisional** | Provisional | NEMA 17 nominal. IMG_5231/IMG_5232 cannot be differenced because IMG_5232's decimals are unreadable |
| `MotorShaftDiameter` | 5.00 mm | **Provisional** | Provisional | IMG_5225 reads `4.9?`. Use 5.00 nominal until the second decimal is confirmed |
| `BearingInnerDiameter` | 8.00 mm | **Provisional** | Provisional | No bearing measured. 8 mm assumes a 608-series |
| `BearingOuterDiameter` | 22.00 mm | **Provisional** | Provisional | No bearing measured |
| `BearingWidth` | 7.00 mm | **Provisional** | Provisional | No bearing measured |
| `CouplerOuterDiameter` | 19.00 mm | **Provisional** | Provisional | No coupler measured |
| `CouplerLength` | 25.00 mm | **Provisional** | Provisional | No coupler measured; drives the tilt-assembly height |
| `CouplerBoreMotorSide` | 5.00 mm | **Provisional** | Provisional | Assumed to match the shaft |
| `CouplerBoreDrivenSide` | 8.00 mm | **Provisional** | Provisional | No coupler measured |
| `InsertOuterDiameter` | 4.60 mm | **Provisional** | Provisional | Common M3 heat-set insert knurl OD. Not measured |
| `InsertLength` | 5.70 mm | **Provisional** | Provisional | Common M3 heat-set insert length. Not measured |
| `InsertPilotDiameter` | 4.00 mm | **Provisional** | Provisional | Pilot hole for the above. Verify against the insert supplier and a test coupon |
| `LimitSwitchBodyLength` | 20.00 mm | **Provisional** | Provisional | No switch measured |
| `LimitSwitchHoleSpacing` | 9.50 mm | **Provisional** | Provisional | No switch measured |
| `AntennaBodyDiameter` | 10.00 mm | **Provisional** | Provisional | No antenna selected or measured |
| `CoaxOuterDiameter` | 3.00 mm | **Provisional** | Provisional | No coax measured. RG316-class assumption |
| `CoaxMinBendRadius` | 15.00 mm | **Provisional** | Provisional | Take from the coax datasheet, not by bending the cable |

## Design choice

Chosen deliberately. No measurement is expected behind these; they are tuning knobs.

| Parameter | Expression | Source | Type | Notes |
|---|---:|---|---|---|
| `WallThickness` | 3.00 mm | Design choice | Design | PETG prototype; increase for the pan base if it flexes |
| `BaseThickness` | 6.00 mm | Design choice | Design | Stationary pan base floor |
| `PrintClearance` | 0.30 mm | Design choice | Design | General part-to-part clearance. Adjust after the first fit coupon |
| `BearingFitAllowance` | 0.05 mm | Design choice | Design | Press-fit interference for a bearing seat. Very printer-dependent — coupon first |
| `HeatsinkAirGap` | 8.00 mm | Design choice | Design | Free air above the TMC2209 heatsink. Do not reduce without a thermal test |
| `CableBendClearance` | 12.00 mm | Design choice | Design | Space behind a motor connector for the plug and wire bend |
| `MountingClearance` | 1.00 mm | Design choice | Design | Slack around fastened interfaces |
| `FilletRadiusStructural` | 3.00 mm | Design choice | Design | Structural fillets at load-bearing intersections |
| `FilletRadiusCosmetic` | 1.00 mm | Design choice | Design | Edge break |
| `RibThickness` | 2.40 mm | Design choice | Design | Stiffening ribs; a multiple of a 0.4 mm nozzle width |
| `TiltAxisHeight` | 90.00 mm | Design choice | Design | Height of the tilt axis above the pan platform. Set for antenna swing clearance |
| `AntennaCentreOffset` | 0.00 mm | Design choice | Design | Target: antenna active centre on the pan/tilt axis intersection. Non-zero is an RF error term |
| `AxisIntersectionOffset` | 0.00 mm | Design choice | Design | Target: pan and tilt axes intersect. Record the real value once geometry is fixed |
| `CounterweightMassTarget` | 0.00 mm | Design choice | Design | Placeholder. Set once the antenna and cradle mass are known |

## How to use this table

1. Create the **Design choice** parameters first — they have no dependencies.
2. Add the **Measured** parameters.
3. Add the **Provisional** parameters, and put the word `PROVISIONAL` in each one's
   Fusion comment field so it is visible in the parameter dialog.
4. Add the **Calculated** parameters last, since they reference the others.
5. Print fit-test coupons for anything that depends on a Provisional value before
   modelling around it.
