# ESP32 development board

Physical measurements of the actual ESP32 board used in Radiance3D Version 1. These
values override any generic "ESP32 DevKit" dimensions found online — devkit outlines
vary widely between vendors.

## Identification

- Component: ESP32 development board with an ESP-WROOM-32 module
- Manufacturer: ELEGOO
- Model or revision: 30-pin ESP32 devkit, USB-C connector, `WIFI+BT SoC Inside`
  `ISM 2.4G 802.11b/g/n` module marking (visible in `IMG_5242.HEIC`)
- Quantity: 1
- Measurement source: photographs IMG_5241–5246
- Measured with: digital caliper, IP54, mm mode
- Units: mm

## Measured dimensions

| Dimension | Value | Measurement source | Notes |
|---|---:|---|---|
| PCB length | 51.47 mm | IMG_5241.HEIC | Long axis, bare board outline |
| PCB width | 28.23 mm | IMG_5242.HEIC | Short axis, bare board outline |
| PCB thickness | 1.33 mm | IMG_5245.HEIC | Bare laminate, measured on a clear board edge |
| Board thickness including top-side components | 4.38 mm | IMG_5246.HEIC | PCB plus the ESP-WROOM-32 module; excludes header pins below |
| USB-C connector width | 8.83 mm | IMG_5243.HEIC | Across the receptacle shell |
| USB-C connector width (repeat) | 8.83 mm | IMG_5244.HEIC | Independent repeat; identical value |

## CAD notes

- **Important protrusions:** the USB-C receptacle on the short end. At 8.83 mm wide
  it needs a cable-access cutout, plus clearance for the plug overmould, which is
  wider than the receptacle. Measure the actual cable's overmould before sizing the
  opening.
- **Board thickness:** 1.33 mm is thinner than the common 1.6 mm assumption. A card
  slot or retention clip sized for 1.6 mm will be loose on this board.
- **Height above board:** 4.38 mm total including the module. Header pins protrude
  below the board and are **not** included in that figure — they are not measured.
- **Mounting-hole layout:** not measured. Mounting holes are visible at the board
  corners in IMG_5241 and IMG_5245 but neither diameter nor spacing was taken. See
  `missing-measurements.md`.
- **Connector clearances:** leave access to the `EN` and `BOOT` buttons, which sit
  beside the USB-C connector.
- **Required print clearance:** the electronics tray should be removable, so size
  the board pocket with clearance rather than an interference fit. See
  [`fusion360-parameters.md`](fusion360-parameters.md).
- **Unclear or missing dimensions:** mounting-hole diameter and spacing, header pin
  length below the board, and USB-C connector protrusion past the board edge. See
  [`missing-measurements.md`](missing-measurements.md).
