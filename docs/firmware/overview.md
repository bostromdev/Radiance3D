# Firmware overview

The planned ESP32 controller owns two motion axes and exposes a host protocol. The
implemented firmware `MotionController` interface covers configured absolute moves,
homing, stop, emergency stop, driver enable, open-loop commanded position, confidence,
limits, and faults. `SimulatedMotionController` is the first implementation. TMC2209
or another physical driver must stay behind the same interface.

The simulator calculates command quantization from axis configuration, applies
configured azimuth/elevation limits, requires homing, and invalidates confidence after
stop, emergency stop, or disable. It does not access GPIO, drive motors, read switches,
apply real acceleration, or detect missed steps. `esp32dev` is a provisional
compilation target, not a statement of supported hardware or pinout.
