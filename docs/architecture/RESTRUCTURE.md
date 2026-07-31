# Repository Restructure

## Goal

Move Radiance3D toward explicit applications, reusable packages, firmware boundaries, simulations, and hardware domains without changing protocol behavior or physical safety behavior during the structural migration.

## Phase 1: Scaffold

- Preserve the validated baseline.
- Create the target directory structure.
- Replace the root README.
- Add architecture documentation.
- Keep current implementation paths functional.
- Run the existing tests.

## Phase 2: Python boundaries

- Separate motion-client code from scan-model code.
- Create an authoritative protocol package.
- Create processing and scan-model packages.
- Add temporary compatibility imports.
- Update packaging and tests.

## Phase 3: Firmware boundaries

- Separate portable controller logic from ESP-IDF integration.
- Establish an explicit shared protocol boundary.
- Move portable firmware tests into the final test location.
- Update CMake and ESP-IDF component paths.
- Preserve simulator and hardware protocol parity.

## Phase 4: Applications

- Add a supported host CLI.
- Add a complete simulator demonstration command.
- Add scan coordination.
- Add visualization applications.

## Phase 5: Cleanup

- Remove deprecated paths.
- Remove compatibility imports.
- Update CI path filters.
- Validate documentation links.
- Run all Python, CTest, simulator-integration, and ESP-IDF builds.

## Validation gate

Every migration commit must preserve:

```bash
cd software
pytest
ruff check .
ruff format --check .
mypy

cd ../firmware/controller
cmake -S host -B build-host -DCMAKE_BUILD_TYPE=Release
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

When ESP-IDF is installed:

```bash
source ~/esp/esp-idf/export.sh
cd firmware/controller
idf.py set-target esp32
idf.py build
idf.py size
```
