# Hardware overview

Version 1 uses an ESP32 development board with an ESP-WROOM-32 module, USB-C, 3.3 V
GPIO logic, and USB serial for host communication. Wi-Fi and Bluetooth are available
on the module but are intentionally unused in Version 1. The motion system uses two
BIGTREETECH TMC2209 V1.3 drivers, each driven over UART with STEP/DIR and firmware-
controlled enable. Two YEJMKJ/LYLANMO NEMA 17 bipolar 4-wire motors form the initial
azimuth and elevation axes.

The current baseline is:

- Controller: ESP32 development board, ESP-WROOM-32, USB serial, 3.3 V logic
- Drivers: BIGTREETECH TMC2209 V1.3, UART controlled, STEP/DIR interface, driver
  diagnostics enabled, current configured in RMS milliamps
- Motors: NEMA 17 bipolar, model `42HDB0014NC-24B`, 1.8° full-step, 200 full
  steps/revolution, 1.0 A rated phase current, 3.5 Ω phase resistance, 3.4 mH phase
  inductance, approximately 0.13 N·m holding torque. Coil A is black + green, coil B
  is red + blue. Nameplate data and measured dimensions are in
  [`Measurements/nema17.md`](../../Measurements/nema17.md)
- Motion: 16 microsteps, direct drive, 3200 microsteps/revolution,
  0.1125° commanded microstep resolution
- Power: 12 V standalone battery, inline fuse, master disconnect, TMC2209 VM power,
  and a regulated 5.0 V buck converter for the ESP32 only

The exact ESP32 board revision, carrier pinout, UART wiring, sense resistor value,
and physical mechanics remain pending validation. See the [Version 1 baseline](../architecture/version-1.md).

## Physical dimensions

Every physical dimension for these components lives in
[`Measurements/`](../../Measurements/README.md), which is the canonical source. This
page deliberately does not repeat those numbers.

- [NEMA 17 stepper motor](../../Measurements/nema17.md)
- [BIGTREETECH TMC2209 V1.3 driver](../../Measurements/tmc2209-v1.3.md)
- [ELEGOO ESP32 devkit](../../Measurements/esp32-devkit.md)

Measured values override generic online dimensions. Dimensions that are still missing
or unclear are tracked in
[`Measurements/missing-measurements.md`](../../Measurements/missing-measurements.md).
The LM2596 buck converters and the RF detector have not been measured.
