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
- Motors: NEMA 17 bipolar, 1.8° full-step, 200 full steps/revolution, 1.0 A rated
  phase current, 3.5 Ω phase resistance, approximately 0.13 N·m holding torque,
  approximately 42 × 42 × 21 mm
- Motion: 16 microsteps, direct drive, 3200 microsteps/revolution,
  0.1125° commanded microstep resolution
- Power: 12 V standalone battery, inline fuse, master disconnect, TMC2209 VM power,
  and a regulated 5.0 V buck converter for the ESP32 only

The exact ESP32 board revision, carrier pinout, UART wiring, sense resistor value,
and physical mechanics remain pending validation. See the [Version 1 baseline](../architecture/version-1.md).
