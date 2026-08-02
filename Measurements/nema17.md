# NEMA 17 stepper motor

Physical measurements of the actual motors used in Radiance3D Version 1. These values
override any generic NEMA 17 dimensions found online.

## Identification

- Component: NEMA 17 bipolar stepper motor
- Manufacturer: YEJMKJ / LYLANMO (marking visible on the motor label in `IMG_5226.HEIC`)
- Model or revision: NEMA 17 bipolar, 1.8° full step, 200 full steps/revolution
- Quantity: 2 (azimuth / pan and elevation / tilt)
- Measurement source: `Measurements/Nema-17/`
- Measured with: digital caliper, IP54, mm mode
- Units: mm

## Measured dimensions

| Dimension | Value | Measurement source | Notes |
|---|---:|---|---|
| Motor body diagonal (corner to corner) | 54.30 mm | IMG_5223.HEIC | Across two opposite chamfered corners of the face plate |
| Motor body length (front face plate to rear face plate) | 20.76 mm | IMG_5231.HEIC | Excludes the front pilot boss; confirms a short-body ("pancake") NEMA 17 |
| Connector protrusion above end-cap face | 9.38 mm | IMG_5226.HEIC | Confirmed by a second shot at a different angle |
| Connector protrusion above end-cap face (repeat) | 9.38 mm | IMG_5227.HEIC | Independent repeat of IMG_5226; same value |
| Motor connector housing length | 16.43 mm | IMG_5228.HEIC | White 6-position polarised housing on the rear end cap |
| Motor face width (across flats) | ≈42 mm | IMG_5221.jpg | **Integer only.** Photo resolution does not resolve the decimals — see `missing-measurements.md` |

## Values read but not yet usable

These display values are legible, but the caliper jaw position in the photo does not
establish which feature was measured, or the decimals are not legible. They are
recorded here for traceability and listed in
[`missing-measurements.md`](missing-measurements.md). **Do not use them in CAD.**

| Reading | Measurement source | Problem |
|---:|---|---|
| illegible (`4?.??`) | IMG_5224.HEIC | LCD blown out by glare; neither digit pair readable |
| 4.9? mm | IMG_5225.HEIC | Jaws are on the output shaft, so this is shaft diameter, but the second decimal is not legible |
| 1.65 mm | IMG_5229.HEIC | Value legible; jaws are near the rear end cap / connector but the feature is not identifiable from the frame |
| 4.00 mm | IMG_5230.HEIC | Value legible; jaws are at the rear corner near the connector, feature not identifiable |
| 22.11 or 22.71 mm | IMG_5232.HEIC | Almost certainly overall length including the front pilot boss, but the two decimal digits cannot be distinguished |

## CAD notes

- **Important protrusions:** the rear connector is the dominant obstruction. It stands
  9.38 mm proud of the end-cap face and its housing is 16.43 mm long. Any pan or tilt
  motor pocket must clear both, plus the mating plug and wire bend radius.
- **Body length:** at 20.76 mm the motor is a short-body NEMA 17. This matches the
  "approximately 42 × 42 × 21 mm" figure recorded in
  [`../docs/architecture/version-1.md`](../docs/architecture/version-1.md). Do not
  design around a 34 mm or 48 mm body.
- **Mounting-hole layout:** not measured. The standard NEMA 17 pattern is 31.0 mm
  square with M3 holes, but this has not been verified on these motors. See
  `missing-measurements.md`.
- **Pilot diameter and height:** not confirmed. The pilot boss is visible in
  IMG_5231/IMG_5232 but the two readings cannot be safely differenced.
- **Shaft:** diameter reads 4.9 mm to one decimal (nominal 5 mm). Length, flat depth,
  and flat length are not measured.
- **Required print clearance:** use a fit-test coupon before committing a bore or
  pocket. See [`fusion360-parameters.md`](fusion360-parameters.md).
- **Unclear or missing dimensions:** see [`missing-measurements.md`](missing-measurements.md).
