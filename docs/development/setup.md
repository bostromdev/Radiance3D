# Development setup

Prerequisites are Git, Python 3.11+, and PlatformIO for firmware work.

```bash
cd software
python3.11 -m venv .venv
source .venv/bin/activate
python -m pip install -e ".[dev]"
pytest
ruff check .
mypy

cd ../firmware/controller
pio run -e native
```

Run `python scripts/check_repository.py` from the repository root. No secrets or paid
services are required.
