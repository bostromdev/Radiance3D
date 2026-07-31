# Radiance3D software

This Python 3.11+ package provides typed scan models plus two intentionally small
commands:

```bash
radiance3d validate path/to/scan.json
radiance3d inspect path/to/scan.json
```

It does not acquire RF data, control physical hardware, or visualize patterns yet.
The validation code enforces the version 1 record shape and domain invariants without
adding a runtime dependency. The JSON Schema remains the normative interchange
specification and is validated separately in CI.
