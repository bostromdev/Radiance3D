# Architecture overview

Radiance3D has an ESP-IDF controller, a portable controller simulator, and a Python
host client. The owned-hardware configuration is generated into the firmware build from
`firmware/config/radiance3d-owned-hardware.json`; hardware boundaries and commissioning
requirements are documented in [the hardware index](../../HARDWARE.md).
