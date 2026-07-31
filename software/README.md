# Radiance3D software

This Python 3.11+ package provides typed scan models, receiver/motion protocols, a
bounds-safe raster planner, a move-settle-measure coordinator, and two intentionally
small commands:

```bash
radiance3d validate path/to/scan.json
radiance3d inspect path/to/scan.json
```

It does not provide a serial transport, RF-device implementation, physical hardware
control, data writer, or visualization yet. The coordinator accepts interchangeable
adapters and preserves raw/rejected readings when computing an aggregate. Validation
enforces schema 1.0 migration reads and the complete 1.1 record invariants without a
runtime dependency. JSON Schema remains the normative interchange specification.
