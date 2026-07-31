# Development setup

Prerequisites are Git, Python 3.11+, and PlatformIO for firmware work.

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
pio run -e native
pio test -e native
pio run -e esp32dev
```

Run `python scripts/check_repository.py` from the repository root. No secrets or paid
services are required. The serial extra is optional for simulator-only work and does
not assume a fixed device path.
