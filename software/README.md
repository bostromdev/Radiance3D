# Radiance3D software

This Python 3.11+ package provides typed scan models, receiver/motion protocols, a
bounds-safe raster planner, a move-settle-measure coordinator, a transport-independent
physical motion client, and two intentionally small commands:

```bash
radiance3d validate path/to/scan.json
radiance3d inspect path/to/scan.json
```

The optional `radiance3d[serial]` extra adds pyserial. `SerialTransport` requires an
explicit device path, validates device identity and protocol version, correlates
commands, queues asynchronous events, rejects mismatched responses, and supports
clean disconnect/reconnect. `PhysicalMotionController` works over that adapter or any
simulator/future transport implementing `ProtocolTransport`.

RF-device integration, a data writer, and visualization are not implemented yet. The
coordinator preserves raw/rejected readings when computing an aggregate. Validation
enforces schema 1.0 migration reads and the complete 1.1 record invariants without a
runtime dependency.
