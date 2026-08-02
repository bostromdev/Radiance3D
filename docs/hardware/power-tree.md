# Power tree

## Verified Hardware

```text
Off-board 12 V bench power source (beneath workbench)
  └─ PWR-000 external two-conductor entry: +12V IN / GND IN
      └─ accessible, strain-relieved side/rear base entry point
          └─ 18 AWG internal distribution point (Red +12 V / Black ground)
              ├─ PWR-001 → TMC2209 pan VM and GND
              ├─ PWR-002 → TMC2209 tilt VM and GND
              ├─ PWR-003 → ZX-052 A VIN+ / VIN−
              │            └─ PWR-005 → regulated 5.0 V → ESP32 5V/VIN and GND
              └─ PWR-004 → ZX-052 B VIN+ / VIN−
                           └─ PWR-006 → regulated 5.0 V → AD8317 +5V and GND

All ground returns meet at the defined common reference. No VOUT+ connection exists
between the two ZX-052 converters.
```

The 12 V battery is a permanent off-board laboratory source, not a Radiance3D part.
The open base provides no battery compartment, battery retention, or battery-weight
allowance. It exposes only the accessible `+12V IN` / `GND IN` entry with appropriate
strain relief before the internal distribution point.

Both buck outputs must measure 5.0 V with a multimeter before loads are attached. The
ESP32 and AD8317 never receive raw 12 V. Keep the RF branch physically separated from
motor VM wiring; run its analog return with the detector harness to the common reference,
not through a motor phase or high-current branch. A recommended (not owned-BOM) fuse and
disconnect belong upstream of the distribution point.

The detector's `VOUT` is a low-voltage electrical signal (`SIG-003`, silicone wire),
not an RF feedline. The AUT threads directly onto the vertically mounted detector SMA;
there is no scanner-mounted RF coax jumper.

## AD8317 acquisition contract

The profile records ADC1 GPIO36, a five-sample median followed by an eight-sample mean,
and calibration coefficients initially `null`. Firmware or host acquisition must store
raw ADC count, converted ADC voltage, timestamp, filter configuration, and calibration
revision. It must label any dBm result as inferred and refuse a numeric conversion until
bench-derived coefficients are committed.
