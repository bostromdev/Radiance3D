# Development setup

Prerequisites are Git, Python 3.11+, CMake for portable firmware tests, and ESP-IDF
v5.5.4 for physical firmware work.

```bash
cd software
python3.11 -m venv .venv
source .venv/bin/activate
python -m pip install -e ".[dev,serial]"
pytest
ruff check .
ruff format --check .
mypy

cd ../firmware/controller
cmake -S host -B build-host -DCMAKE_BUILD_TYPE=Release
cmake --build build-host
ctest --test-dir build-host --output-on-failure
```

For the physical target, install and export Espressif's pinned release, then run:

```bash
cd firmware/controller
idf.py set-target esp32
idf.py build
idf.py size
```

Run `python scripts/check_repository.py` from the repository root. CI also runs the
Python host client against the CMake-built simulator. No secrets or paid services are
required. The serial extra is optional for simulator-only work and does not assume a
fixed device path.
