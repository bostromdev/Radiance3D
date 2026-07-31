# Firmware overview

The planned ESP32 controller owns two motion axes and exposes a host protocol. Axis
drivers should be replaceable behind interfaces for move, home/zero, stop, position,
limit state, and fault state.

The current implementation is an in-memory simulator. It does not access GPIO,
drivers, encoders, or limit switches. `esp32dev` is a provisional compilation target,
not a statement of supported hardware.
