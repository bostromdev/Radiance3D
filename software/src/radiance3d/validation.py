"""Loading and error handling for Radiance3D scan data."""

from __future__ import annotations

import json
from pathlib import Path

from radiance3d.models import Scan


class ScanValidationError(ValueError):
    """A user-facing scan parsing or validation error."""


def load_scan(path: str | Path) -> Scan:
    """Load and validate the core invariants of a version 1 scan."""

    source = Path(path)
    try:
        raw = json.loads(source.read_text(encoding="utf-8"))
    except OSError as exc:
        raise ScanValidationError(f"cannot read {source}: {exc}") from exc
    except json.JSONDecodeError as exc:
        raise ScanValidationError(
            f"{source}:{exc.lineno}:{exc.colno}: invalid JSON: {exc.msg}"
        ) from exc

    try:
        return Scan.from_mapping(raw)
    except ValueError as exc:
        raise ScanValidationError(f"{source}: {exc}") from exc
