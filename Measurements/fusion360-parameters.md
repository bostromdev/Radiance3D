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

## The one rule: never edit a measured value to fix a fit

Every critical dimension exists twice:

| | Parameter | Changes when |
|---|---|---|
| **Measured value** | e.g. `mMotorPilotHeight` | Never — only if the part is re-measured or replaced |
| **CAD value** | e.g. `cMotorPilotPocketDepth` | Freely — it is an expression, and fit is tuned through the clearance term |

```text
mMotorPilotHeight        = 2.00 mm                              <- measured, frozen
cMotorPilotPocketDepth   = mMotorPilotHeight + dPrintClearance    <- CAD, tune via clearance
```

When a printed part comes out too tight, the fix is `dPrintClearance` or
`dBearingFitAllowance` — **never** the measured number. Editing a measured value to fix
a fit destroys the record of what the hardware actually is, and the error then
propagates silently into every other feature that references it.

This is why the fit parameters are separate, single-purpose, and few: `dPrintClearance`,
`dBearingFitAllowance`, `dMountingClearance`, `dHeatsinkAirGap`, `dCableBendClearance`. All
tuning happens in those.

## Naming convention

Parameter names carry their type as a prefix, matching what Fusion Assistant generates,
so the repository and the Fusion parameter table use identical names:

| Prefix | Type | Example |
|---|---|---|
| `m` | Measured | `mMotorPilotDiameter` |
| `c` | Calculated | `cMotorPilotSeatDiameter` |
| `d` | Design choice | `dPrintClearance` |
| `p` | Provisional | `pBearingOuterDiameter` |

The prefix is visible at every use site, so a provisional value cannot be mistaken for
a measured one inside an expression.

## Fit policy — PETG, forgiving

Target fit is a slip fit at **+0.15 mm per side** (`dPrintClearance`), giving 0.30 mm
total across a bore or slot.

**0.04 mm per side was specified, and has been overridden to 0.15 mm.** That is a
judgement call, and it is one line to revert.

Reason: 0.04 mm per side is machining-grade, not FFF-grade. PETG bores print undersized
by roughly 0.1–0.3 mm on diameter from die swell and perimeter pull-in, *before* any
clearance is applied. The printer eats 0.04 mm before a fit exists, so the result is an
interference fit needing force or a reamer — the opposite of forgiving. 0.15 mm per
side (0.30 mm total) is a standard PETG slip fit that still assembles by hand.

The register coupon settles it in one print. Change `dPrintClearance`, never a
measured value.

> [!CAUTION]
> **Shallow features sit inside the elephant-foot zone.** The motor pilot register is
> only 2.00 mm deep. Printed face-down, the whole feature is within the first few
> layers, where PETG squishes outward and shrinks the bore — tight at the bottom,
> correct at the top, and the motor rocks. Print that face **up**, or rely on
> `dBoreMouthChamfer`. State the print orientation for every part with a shallow
> register.

Fit classes are deliberately separate, because they are not the same problem:

| Parameter | Fit class | Sign | Applies to |
|---|---|---|---|
| `dPrintClearance` | Slip fit | Positive — adds material clearance | Motor pilot register, board pockets, card slots, general part-to-part |
| `dBearingFitAllowance` | Press fit | Zero — bore at nominal; the printer's undersizing supplies the interference | Bearing outer-race seats only |
| `dMountingClearance` | Loose | Positive, generous | Fastened interfaces where position is set by the fastener |

Never apply `dPrintClearance` to a bearing seat. A bearing that slips into its housing
is a failed bearing seat.

---

## Measured

Traceable to a caliper reading and a photograph.

| Parameter | Expression | Source | Type | Notes |
|---|---:|---|---|---|
| `mMotorBodyDiagonal` | 54.30 mm | nema17.md — IMG_5223 | Measured | Corner to corner across chamfered corners |
| `mMotorBodyLength` | 20.84 mm | nema17.md — IMG_5249 | Measured | Front face plate to rear face plate, excludes pilot boss. IMG_5231 gave 20.76 mm |
| `mMotorLengthWithBoss` | 22.85 mm | nema17.md — IMG_5248 | Measured | Pilot boss face to rear face |
| `mMotorOverallLength` | 45.18 mm | nema17.md — IMG_5250 | Measured | Shaft tip to rear face — the full axial envelope |
| `mMotorPilotDiameter` | 21.97 mm | nema17.md — IMG_5247 | Measured | Register that centres the motor. Was provisional at 22.00 mm |
| `mMotorPilotHeight` | 2.00 mm | nema17.md — derived, ±0.06 mm | Measured | Derivation gives 1.95 and 2.01 by two routes; 2.00 adopted. **Frozen — tune fit via `dPrintClearance`** |
| `mAd8317OverallWidth` | 55.84 mm | ad8317.md — IMG_5257, IMG_5258 | Measured | SMA tip to tip, along the SMA axis |
| `mAd8317PcbLength` | 36.27 mm | ad8317.md — IMG_5261 | Measured | Bare PCB, along the SMA axis |
| `mAd8317PcbWidth` | 35.72 mm | ad8317.md — IMG_5262 | Measured | Bare PCB, perpendicular. **Board is not square** |
| `mAd8317ShieldCanLength` | 17.36 mm | ad8317.md — IMG_5259, IMG_5260 | Measured | RF shield can only; mean of 17.34 / 17.38 |
| `mMotorConnectorProtrusion` | 9.38 mm | nema17.md — IMG_5226, IMG_5227 | Measured | Height of rear connector above end-cap face; two independent readings agreed |
| `mMotorConnectorLength` | 16.43 mm | nema17.md — IMG_5228 | Measured | 6-position housing on the rear end cap |
| `mMotorShaftLengthFromBoss` | 22.39 mm | nema17.md — operator measurement | Measured | **Datum is the pilot boss face, not the motor face plate.** No source photograph |
| `mDriverPcbLength` | 20.14 mm | tmc2209-v1.3.md — IMG_5234 | Measured | Bare PCB long axis |
| `mDriverPcbWidth` | 15.14 mm | tmc2209-v1.3.md — IMG_5233, IMG_5238 | Measured | Bare PCB short axis; repeat read 15.16 mm |
| `mDriverHeatsinkLeftToPcbRight` | 12.21 mm | tmc2209-v1.3.md — IMG_5235 | Measured | Heatsink left face to far PCB edge |
| `mDriverHeatsinkRightToPcbLeft` | 11.22 mm | tmc2209-v1.3.md — IMG_5236 | Measured | Heatsink right face to far PCB edge |
| `mDriverHeatsinkFrontToPcbBack` | 13.73 mm | tmc2209-v1.3.md — IMG_5237 | Measured | Heatsink front face to far PCB edge |
| `mDriverInstalledHeight` | 22.24 mm | tmc2209-v1.3.md — IMG_5239 | Measured | Heatsink top to header-pin tips; full envelope |
| `mEsp32PcbLength` | 51.47 mm | esp32-devkit.md — IMG_5241 | Measured | Bare board outline |
| `mEsp32PcbWidth` | 28.23 mm | esp32-devkit.md — IMG_5242 | Measured | Bare board outline |
| `mEsp32PcbThickness` | 1.33 mm | esp32-devkit.md — IMG_5245 | Measured | Thinner than the usual 1.6 mm assumption |
| `mEsp32BoardHeightWithModule` | 4.38 mm | esp32-devkit.md — IMG_5246 | Measured | PCB plus ESP-WROOM-32; excludes header pins below |
| `mEsp32UsbCWidth` | 8.83 mm | esp32-devkit.md — IMG_5243, IMG_5244 | Measured | Receptacle shell only; plug overmould is wider |

## Calculated

Derived by expression. Fusion will keep these correct when their inputs change.

| Parameter | Expression | Source | Type | Notes |
|---|---:|---|---|---|
| `cMotorPocketDepth` | `mMotorBodyLength + dPrintClearance` | Calculated | Calculated | Pocket for the motor body only |
| `cMotorRearClearance` | `mMotorConnectorProtrusion + dCableBendClearance` | Calculated | Calculated | Space behind the motor for the connector and its plug. 21.38 mm at current values — this decides whether the pan motor can face shaft-up |
| `cDriverBayHeight` | `mDriverInstalledHeight + dHeatsinkAirGap` | Calculated | Calculated | Never make the bay exactly `mDriverInstalledHeight`; the heatsink needs air |
| `cDriverHeatsinkWidth` | `mDriverHeatsinkLeftToPcbRight + mDriverHeatsinkRightToPcbLeft - mDriverPcbWidth` | Calculated | Calculated | 8.29 mm. Fully measured — no provisional input |
| `cDriverHeatsinkOffset` | `(mDriverHeatsinkLeftToPcbRight - mDriverHeatsinkRightToPcbLeft) / 2` | Calculated | Calculated | 0.495 mm from the PCB centre. This is why the driver is handed |
| `cDriverHeatsinkFrontEdge` | `mDriverPcbLength - mDriverHeatsinkFrontToPcbBack` | Calculated | Calculated | 6.41 mm from the PCB front edge. The heatsink's **back** edge is unknown — see `missing-measurements.md` |
| `cEsp32PocketLength` | `mEsp32PcbLength + 2 * dPrintClearance` | Calculated | Calculated | Tray pocket, removable fit |
| `cEsp32PocketWidth` | `mEsp32PcbWidth + 2 * dPrintClearance` | Calculated | Calculated | Tray pocket, removable fit |
| `cMotorFaceHalf` | `pMotorFaceWidth / 2` | Calculated | Calculated | Used to centre the motor on its mount |
| `cMotorPilotPocketDepth` | `mMotorPilotHeight + dPrintClearance` | Calculated | Calculated | The counterbore in the printed mount. Tune fit here, never in `mMotorPilotHeight` |
| `cMotorPilotSeatDiameter` | `mMotorPilotDiameter + dPrintClearance` | Calculated | Calculated | The register bore in the printed mount |
| `cDriverBayLength` | `mDriverPcbLength + 2 * dPrintClearance` | Calculated | Calculated | Driver bay footprint, removable fit |
| `cDriverBayWidth` | `mDriverPcbWidth + 2 * dPrintClearance` | Calculated | Calculated | Driver bay footprint, removable fit |
| `cEsp32SlotThickness` | `mEsp32PcbThickness + dPrintClearance` | Calculated | Calculated | Card-slot width. The board is 1.33 mm, not 1.6 mm |
| `cAd8317SmaProtrusion` | `(mAd8317OverallWidth - mAd8317PcbLength) / 2` | Calculated | Calculated | 9.78 mm per side. Normal for an edge-launch SMA, which corroborates the two readings |
| `cAd8317PocketLength` | `mAd8317PcbLength + 2 * dPrintClearance` | Calculated | Calculated | Removable holder, not an interference fit — this board carries RF |
| `cAd8317PocketWidth` | `mAd8317PcbWidth + 2 * dPrintClearance` | Calculated | Calculated | Removable holder |
| `cMotorShaftLengthFromFace` | `mMotorShaftLengthFromBoss + mMotorPilotHeight` | Calculated | Calculated | 24.40 mm — the vendor-style figure. Now fully derived from measured values |
| `cCouplerShaftEngagement` | `pCouplerLength / 2` | Calculated | Calculated | Motor-side engagement. Must stay below `mMotorShaftLengthFromBoss` (22.39 mm) |
| `cBearingSeatBore` | `pBearingOuterDiameter - dBearingFitAllowance` | Calculated | Calculated | Press-fit seat; validate with a coupon first |

## Provisional — must be replaced before printing for fit

These are placeholders. Each one has an entry in
[`missing-measurements.md`](missing-measurements.md).

| Parameter | Expression | Source | Type | Notes |
|---|---:|---|---|---|
| `pMotorFaceWidth` | 42.30 mm | **Provisional** | Provisional | IMG_5221 reads `42` but decimals are unreadable. 42.30 is the NEMA 17 nominal, not a measurement |
| `pMotorHoleSpacing` | 31.00 mm | **Provisional** | Provisional | NEMA 17 nominal square pattern. Not verified on these motors |
| `pMotorHoleDiameter` | 3.40 mm | **Provisional** | Provisional | M3 clearance, assumed. Not measured |
| `pMotorShaftDiameter` | 5.00 mm | **Provisional** | Provisional | IMG_5225 reads `4.9?`. Use 5.00 nominal until the second decimal is confirmed |
| `pBearingInnerDiameter` | 8.00 mm | **Provisional** | Provisional | No bearing measured. 8 mm assumes a 608-series |
| `pBearingOuterDiameter` | 22.00 mm | **Provisional** | Provisional | No bearing measured |
| `pBearingWidth` | 7.00 mm | **Provisional** | Provisional | No bearing measured |
| `pCouplerOuterDiameter` | 19.00 mm | **Provisional** | Provisional | No coupler measured |
| `pCouplerLength` | 25.00 mm | **Provisional** | Provisional | No coupler measured; drives the tilt-assembly height |
| `pCouplerBoreMotorSide` | 5.00 mm | **Provisional** | Provisional | Assumed to match the shaft |
| `pCouplerBoreDrivenSide` | 8.00 mm | **Provisional** | Provisional | No coupler measured |
| `pInsertOuterDiameter` | 4.60 mm | **Provisional** | Provisional | Common M3 heat-set insert knurl OD. Not measured |
| `pInsertLength` | 5.70 mm | **Provisional** | Provisional | Common M3 heat-set insert length. Not measured |
| `pInsertPilotDiameter` | 4.00 mm | **Provisional** | Provisional | Pilot hole for the above. Verify against the insert supplier and a test coupon |
| `pLimitSwitchBodyLength` | 20.00 mm | **Provisional** | Provisional | No switch measured |
| `pLimitSwitchHoleSpacing` | 9.50 mm | **Provisional** | Provisional | No switch measured |
| `pAntennaBodyDiameter` | 10.00 mm | **Provisional** | Provisional | No antenna selected or measured |
| `pCoaxOuterDiameter` | 3.00 mm | **Provisional** | Provisional | No coax measured. RG316-class assumption |
| `pCoaxMinBendRadius` | 15.00 mm | **Provisional** | Provisional | Take from the coax datasheet, not by bending the cable |

## Design choice

Chosen deliberately. No measurement is expected behind these; they are tuning knobs.

| Parameter | Expression | Source | Type | Notes |
|---|---:|---|---|---|
| `dWallThickness` | 3.00 mm | Design choice | Design | PETG prototype; increase for the pan base if it flexes |
| `dBaseThickness` | 6.00 mm | Design choice | Design | Stationary pan base floor |
| `dPrintClearance` | 0.15 mm | Design choice | Design | Slip fit, **per side** (0.30 mm total across a bore). PETG starting value. Was specified at 0.04 mm — see the fit-policy note |
| `dBearingFitAllowance` | 0.00 mm | Design choice | Design | Bore modelled at the exact bearing OD. FFF already undersizes holes, which supplies the interference — do not stack a second one. Never use `dPrintClearance` here |
| `dHeatsinkAirGap` | 8.00 mm | Design choice | Design | Free air above the TMC2209 heatsink. Do not reduce without a thermal test |
| `dCableBendClearance` | 12.00 mm | Design choice | Design | Space behind a motor connector for the plug and wire bend |
| `dMountingClearance` | 1.00 mm | Design choice | Design | Slack around fastened interfaces |
| `dFilletRadiusStructural` | 3.00 mm | Design choice | Design | Structural fillets at load-bearing intersections |
| `dFilletRadiusCosmetic` | 1.00 mm | Design choice | Design | Edge break |
| `dRibThickness` | 2.40 mm | Design choice | Design | Stiffening ribs; a multiple of a 0.4 mm nozzle width |
| `dBoreMouthChamfer` | 0.50 mm | Design choice | Design | Chamfer at the mouth of every bore and register, so squished first layers have somewhere to go |
| `dTiltAxisHeight` | 90.00 mm | Design choice | Design | Height of the tilt axis above the pan platform. Set for antenna swing clearance |
| `dAntennaCentreOffset` | 0.00 mm | Design choice | Design | Target: antenna active centre on the pan/tilt axis intersection. Non-zero is an RF error term |
| `dAxisIntersectionOffset` | 0.00 mm | Design choice | Design | Target: pan and tilt axes intersect. Record the real value once geometry is fixed |
| `dCounterweightMassTarget` | 0.00 mm | Design choice | Design | Placeholder. Set once the antenna and cradle mass are known |

## How to use this table

1. Create the **Design choice** parameters first — they have no dependencies.
2. Add the **Measured** parameters.
3. Add the **Provisional** parameters, and put the word `PROVISIONAL` in each one's
   Fusion comment field so it is visible in the parameter dialog.
4. Add the **Calculated** parameters last, since they reference the others.
5. Print fit-test coupons for anything that depends on a Provisional value before
   modelling around it.
