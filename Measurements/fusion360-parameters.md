# Fusion 360 parameters — Revision 1

Revision 1 is an open, direct-drive desktop prototype within a 220 × 220 mm maximum
base. Exact placement is a Fusion layout-study result.

## Naming and fit policy

| Prefix | Meaning |
|---|---|
| `m...` | physically measured and frozen |
| `c...` | calculated expression |
| `d...` | adjustable design choice |
| `p...` | provisional value, explicitly labelled `PROVISIONAL` |

Never edit a measured value to fix print fit. Tune `dPrintClearance`,
`dHubProfileClearance`, or another named design allowance. Start PETG slip fits at
0.15 mm per side and verify the motor pilot and D-shaft profiles with coupons.

## Measured

| Parameter | Value | Interpretation |
|---|---:|---|
| `mMotorBodyDiagonal` | 54.30 mm | chamfered-corner envelope |
| `mMotorFaceEnvelopeMax` | 43.46 mm | larger photographed outer-face envelope; clearance only |
| `mMotorBodyLengthSafe` | 20.84 mm | larger same-face reading; 20.72 mm repeat retained in `nema17.md` |
| `mMotorLengthWithBoss` | 22.85 mm | boss face to rear face |
| `mMotorOverallLength` | 45.18 mm | shaft tip to rear face |
| `mMotorPilotDiameter` | 21.97 mm | precision locating register |
| `mMotorPilotHeight` | 2.00 mm | adopted derived value, ±0.06 mm |
| `mMotorHoleSpacing` | 31.00 mm | measured one axis; nominally square pattern |
| `mMotorShaftDiameter` | 5.00 mm | D-shaft major diameter |
| `mMotorDFlatOppositeThickness` | 4.60 mm | flat surface to opposite round surface |
| `mMotorShaftLengthFromBoss` | 22.39 mm | boss-face datum |
| `mMotorConnectorProtrusionMax` | 9.53 mm | larger repeat used for clearance |
| `mMotorConnectorHousingEnvelope` | 16.44 mm | latest repeat |
| `mDriverPcbLength` | 20.14 mm | TMC2209 PCB |
| `mDriverPcbWidth` | 15.14 mm | TMC2209 PCB |
| `mDriverInstalledHeight` | 22.24 mm | heatsink top to pin tips |
| `mEsp32PcbLength` | 51.47 mm | PCB envelope |
| `mEsp32PcbWidth` | 28.23 mm | PCB envelope |
| `mEsp32PcbThickness` | 1.33 mm | bare PCB |
| `mEsp32BoardHeightWithModule` | 4.38 mm | excludes lower header pins |
| `mEsp32UsbCWidth` | 8.83 mm | receptacle shell only |
| `mZX052PcbLength` | 66.07 mm | each of two boards |
| `mZX052PcbWidth` | 36.48 mm | each of two boards |
| `mZX052EdgeThickness` | 2.15 mm | local edge only, not installed height |
| `mAd8317OverallWidth` | 55.84 mm | SMA tip to tip |
| `mAd8317PcbLength` | 36.27 mm | along SMA axis |
| `mAd8317PcbWidth` | 35.72 mm | perpendicular to SMA axis |
| `mAd8317ShieldCanLength` | 17.36 mm | shield can only |

## Calculated

| Parameter | Expression |
|---|---|
| `cMotorDFlatRadialDepth` | `(mMotorShaftDiameter - mMotorDFlatOppositeThickness) / 2` |
| `cMotorPilotSeatDiameter` | `mMotorPilotDiameter + 2 * dPrintClearance` |
| `cMotorPilotPocketDepth` | `mMotorPilotHeight + dPrintClearance` |
| `cMotorBodyClearanceEnvelope` | `mMotorFaceEnvelopeMax + 2 * dPrintClearance` |
| `cMotorRearClearance` | `mMotorConnectorProtrusionMax + dCableBendClearance` |
| `cDHubMajorProfile` | `mMotorShaftDiameter + 2 * dHubProfileClearance` |
| `cDHubFlatProfile` | `mMotorDFlatOppositeThickness + dHubProfileClearance` |
| `cDriverBayLength` | `mDriverPcbLength + 2 * dPrintClearance` |
| `cDriverBayWidth` | `mDriverPcbWidth + 2 * dPrintClearance` |
| `cDriverBayHeight` | `mDriverInstalledHeight + dHeatsinkAirGap` |
| `cEsp32TrayLength` | `mEsp32PcbLength + 2 * dPrintClearance` |
| `cEsp32TrayWidth` | `mEsp32PcbWidth + 2 * dPrintClearance` |
| `cZX052TrayLength` | `mZX052PcbLength + 2 * dPrintClearance` |
| `cZX052TrayWidth` | `mZX052PcbWidth + 2 * dPrintClearance` |
| `cAd8317SmaProtrusion` | `(mAd8317OverallWidth - mAd8317PcbLength) / 2` |
| `cAd8317TrayLength` | `mAd8317PcbLength + 2 * dPrintClearance` |
| `cAd8317TrayWidth` | `mAd8317PcbWidth + 2 * dPrintClearance` |

## Design choices

| Parameter | Starting value | Purpose |
|---|---:|---|
| `dBaseMaximumWidth` | 220.00 mm | hard desktop boundary |
| `dBaseMaximumDepth` | 220.00 mm | hard desktop boundary |
| `dBaseThickness` | 6.00 mm | PETG starting plate thickness |
| `dWallThickness` | 3.00 mm | trays and mounts |
| `dPrintClearance` | 0.15 mm | slip-fit clearance per side |
| `dHubProfileClearance` | 0.10 mm | coupon-tuned D-profile allowance |
| `dHubClampGap` | 1.20 mm | split-clamp starting gap |
| `dMountingClearance` | 1.00 mm | loose serviceable joints |
| `dHeatsinkAirGap` | 8.00 mm | free air above TMC2209 |
| `dCableBendClearance` | 12.00 mm | connector wire bend |
| `dBoreMouthChamfer` | 0.50 mm | PETG bore/register entry |
| `dFilletRadiusStructural` | 3.00 mm | load-bearing transitions |
| `dRibThickness` | 2.40 mm | 0.4 mm-nozzle multiple |
| `dTiltAxisHeight` | 90.00 mm | adjustable layout-study start |
| `dAntennaCentreOffset` | 0.00 mm | target phase-centre offset |
| `dAxisIntersectionOffset` | 0.00 mm | target pan/tilt-axis offset |

Do not freeze a pan-platform diameter or electronics coordinates before the layout
study proves them.

## Provisional, non-blocking

| Parameter | Starting value | Use |
|---|---:|---|
| `pMotorHoleDiameter` | 3.50 mm | editable M3-compatible prototype clearance |
| `pHubEngagementLength` | 12.00 mm | adjustable; do not hard-stop on unknown flat length |
| `pInsertOuterDiameter` | 4.60 mm | optional M3 insert coupon only |
| `pInsertLength` | 5.70 mm | optional M3 insert coupon only |
| `pZX052OpenHeight` | 25.00 mm | conservative open keep-out, not a measured height |
| `pAUTClearanceDiameter` | 45.00 mm | adjustable representative envelope |
| `pAUTClearanceLength` | 160.00 mm | adjustable representative envelope |
| `pHarnessBundleDiameter` | 10.00 mm | flexible swept envelope |
| `pHarnessBendRadius` | 25.00 mm | conservative motion-study start |

The provisional values support reversible prototype geometry. Exact PCB holes, an AUT
body, and final harness length are not prerequisites for the first CAD layout.
