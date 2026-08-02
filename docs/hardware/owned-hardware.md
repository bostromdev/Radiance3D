# Owned hardware manifest and BOM

The JSON hardware profile is the machine-readable source of truth. Photo filenames
are relative to `Part Numbers:Views/`; a missing photo is an audit finding, not an
invitation to substitute a generic render.

The [Fusion reference architecture](reference-architecture.md) is the official
**Design Intent** baseline. Revision 1 is an open desktop prototype, not an enclosure;
Fusion determines final dimensions and component locations. Everything below is
**Verified Hardware** unless explicitly labelled Design Intent.

| Item | Manufacturer / model | Qty | Electrical role | Photo | Firmware / harness |
|---|---|---:|---|---|---|
| Controller | ELEGOO ESP32 DevKit-style, ESP-WROOM-32, USB-C, rear OLED | 1 | 5 V input, 3.3 V logic | `ESP-32-elegoo top view(part #EL-SM-012).jpeg` | PWR-005; SIG-001–004 |
| Driver | BIGTREETECH TMC2209 V1.3 | 2 | 12 V VM, STEP/DIR/PDN_UART | `TMC2209 v1.3 top view with heatsink.jpeg` | PWR-001/002; SIG-001/002; MTR-001/002 |
| Motor | YEJMKJ 42HDB0014NC | 2 | 1.0 A/phase bipolar | `NEMA-17(part #YEJMKJ).jpeg` | MTR-001, MTR-002 |
| RF detector | AD8317 EVAL BD / NWDZ V1.0 | 1 | regulated 5 V, analog output, direct-SMA AUT interface | Supplemental measurement photos supplied; not in repository photo path | PWR-006; SIG-003 |
| Buck A | ZX-052 V2.0, PCB 66.07 × 36.48 mm | 1 | 12 V to 5.0 V digital branch | Supplemental photos supplied; not in repository photo path | PWR-003, PWR-005 |
| Buck B | ZX-052 V2.0, PCB 66.07 × 36.48 mm | 1 | 12 V to 5.0 V RF branch | Supplemental photos supplied; not in repository photo path | PWR-004, PWR-006 |
| External power interface | +12V IN / GND IN from off-board 12 V bench source | 1 | strain-relieved enclosure entry | **No photo recorded** | PWR-000 |

| Cable | Model | Qty | Permitted use | Harness namespace |
|---|---|---:|---|---|
| Silicone wire | 18/22/26 AWG, five colors | owned stock | all DC, motor, GPIO, and low-voltage signals | PWR, SIG, MTR |
| RF coax | RG316, 50 Ω | owned stock | RF transmitter, antenna, detector input, 50 Ω interconnects only | RF |

No fuse, switch, connector, clamp, or protection component is included in this owned-parts
BOM. An inline fuse and accessible disconnect are recommended safety additions only.
The external bench source is not a scanner component and has no base allocation.

## Component facts

- Each motor is 42 × 42 × 21 mm, 1.8°/step (200 full steps/rev), 3.5 Ω/phase,
  1.0 A/phase, approximately 0.13 N·m holding torque. Label wiring is Black A+,
  Green A−, Red B+, Blue B−.
- Both drivers are TMC2209 V1.3. Production control is STEP/DIR with enable and
  PDN/UART diagnostics. The heatsink is installed only after inspection, before
  powered motion testing.
- The AD8317 is powered from the isolated RF/analog 5 V branch. Its output is measured
  on ADC1 GPIO36; raw counts and volts are measurements, while dBm is an inference
  requiring a saved bench calibration.
- **Design Intent:** the external stationary 5.8 GHz VTX is not mounted on the scanner.
  The AUT is intended to screw directly onto the AD8317 SMA; no antenna-to-detector
  RG316 jumper is in the baseline.
- The two motors directly drive lightweight Revision-1 loads through removable clamp
  hubs on their measured 5 mm D-shafts. No external bearings, separate shafts, or
  couplers are Revision-1 parts.
- Unknown PCB hole patterns do not block CAD. Use open reversible board retention and
  preserve every terminal, display, adjustment, cooling, and wiring-access direction.

Datasheets are external reference material: ESP32-WROOM-32 datasheet, Trinamic
TMC2209 datasheet, AD8317 datasheet, and each board maker's documentation must be
consulted before a value not visible in the photos is used. In particular, do not infer
ZX-052 current capability, AD8317 connector pin order, or OLED wiring from this table.
