# Repository layout

The monorepo keeps contracts close to their consumers:

- `Measurements/` is canonical for every physical dimension, and holds the source
  photographs. Other documents link to it instead of repeating dimensions.
- `data/schemas/` is normative for stored datasets.
- `software/` consumes the schema and implements host-side domain behavior.
- `firmware/` owns the controller protocol implementation.
- `docs/` explains cross-component contracts and provisional designs.
- `hardware/` will hold editable design sources only when they exist.
- `scripts/` checks the repository itself; `tools/` is for dataset utilities.
- `assets/` contains only documented, intentional project media.

Generated builds, raw captures, temporary CAD files, and exports are ignored.
