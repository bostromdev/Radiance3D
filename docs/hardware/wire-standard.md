# Electrical and RF cabling standard

## Verified Hardware

Radiance3D has two non-interchangeable cabling systems.

- **Silicone wire** carries every DC, motor, and low-voltage signal. Available sizes
  are 18, 22, and 26 AWG in Black, Red, White, Yellow, and Blue.
- **RG316 50 Ω coax** carries RF signal paths only. Every RF drawing depicts its centre
  conductor and shield, never an ordinary two-conductor wire.

Silicone wire must never replace RG316 for an RF path. RG316 must never carry power,
ground distribution, GPIO, UART, STEP, DIR, EN, ADC, E-stop, limit-switch, or motor
connections. Color is never a sole identifier: every conductor has a harness ID,
signal name, from/to endpoint, and labels at both ends.

## Silicone-wire standard

| Gauge | Approved use | Color convention |
|---|---|---|
| 18 AWG | external 12 V entry; internal distribution-to-buck inputs; practical VM/GND branches | Red +12 V; Black power ground |
| 22 AWG | motor extensions; regulated 5 V/GND branches | motor: Black A+, Yellow A− labelled `A- / MOTOR GREEN`, Red B+, Blue B−; supply: Red +5 V, Black GND |
| 26 AWG | STEP, DIR, EN, PDN_UART, ADC, E-stop, future limit switches and diagnostics | White primary data/STEP/UART; Yellow DIR/EN; Blue analog/auxiliary; Black signal ground |

## Electrical harnesses — silicone wire

| Harness | Endpoints and signals | Gauge / color | Required labels |
|---|---|---|---|
| PWR-000 | off-board bench source → open-base `+12V IN` / `GND IN` entry | 18 AWG Red/Black | `+12V IN`, `GND IN`, source/base |
| PWR-001 | distribution → pan driver VM/GND | 18 AWG Red/Black | `PWR-001 +12V`, `PWR-001 GND` |
| PWR-002 | distribution → tilt driver VM/GND | 18 AWG Red/Black | `PWR-002 +12V`, `PWR-002 GND` |
| PWR-003 | distribution → Buck A VIN+/VIN− | 18 AWG Red/Black | endpoint labels |
| PWR-004 | distribution → Buck B VIN+/VIN− | 18 AWG Red/Black | endpoint labels |
| PWR-005 | Buck A VOUT+/VOUT− → ESP32 5V/VIN/GND | 22 AWG Red/Black | `5V`, `GND`, source/destination |
| PWR-006 | Buck B VOUT+/VOUT− → AD8317 +5V/GND | 22 AWG Red/Black | `5V`, `GND`, source/destination |
| SIG-001 | GPIO25 STEP, 26 DIR, 27 ENN, 17 TX, 16 RX, GND → pan driver | 26 AWG | each signal plus endpoints |
| SIG-002 | GPIO18 STEP, 19 DIR, 23 ENN, 14 TX, 39 RX, GND → tilt driver | 26 AWG | each signal plus endpoints |
| SIG-003 | AD8317 VOUT/GND → GPIO36/GND | 26 AWG Blue/Black | `AD8317 VOUT → GPIO36`, `AGND` |
| SIG-004 | future E-stop or limit switch only | 26 AWG | signal, endpoints, pull configuration |
| MTR-001 | pan driver A1/A2/B1/B2 → motor Black/Yellow/Red/Blue | 22 AWG Black/Yellow/Red/Blue | `A+`, `A- / MOTOR GREEN`, `B+`, `B-` |
| MTR-002 | tilt driver A1/A2/B1/B2 → motor Black/Yellow/Red/Blue | 22 AWG Black/Yellow/Red/Blue | `A+`, `A- / MOTOR GREEN`, `B+`, `B-` |

Motor terminal mapping is mandatory: A1/A+ → Black, A2/A− → Yellow extension
(motor Green), B1/B+ → Red, B2/B− → Blue. Never connect or disconnect a motor while
its driver is powered.

## RF harnesses — RG316 50 Ω coax

| Harness | RF path | Construction and drawing rule | Status |
|---|---|---|---|
| RF-001 | external stationary VTX output → its external transmit antenna | RG316 centre conductor is RF signal; braided shield is the RF return/shield | off-scanner path; connector/endpoints require bench confirmation |

The AUT threads directly onto the vertically mounted AD8317 SMA: there is no baseline
RG316 jumper or `RF-002` path between antenna and detector. Do not use the AD8317
`VOUT` analog connection as an RF harness: it is `SIG-003` silicone wire. Any future
RF path receives the next `RF-###` identifier and is drawn as coax with separate centre
and shield representations.
