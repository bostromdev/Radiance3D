import json
from pathlib import Path

import pytest

from radiance3d.models import Angle
from radiance3d.validation import ScanValidationError, load_scan

EXAMPLE = Path(__file__).parents[2] / "data" / "examples" / "simulated" / "dipole-like-scan.json"


def test_simulated_example_loads() -> None:
    scan = load_scan(EXAMPLE)
    assert scan.data_kind == "simulated"
    assert len(scan.samples) == 5
    assert scan.samples[0].measurement.unit == "dB_relative"


def test_angle_rejects_out_of_range_value() -> None:
    with pytest.raises(ValueError, match="azimuth angle"):
        Angle("azimuth", 361.0)


def test_invalid_json_has_context(tmp_path: Path) -> None:
    invalid = tmp_path / "invalid.json"
    invalid.write_text("{", encoding="utf-8")
    with pytest.raises(ScanValidationError, match="invalid JSON"):
        load_scan(invalid)


def test_invalid_firmware_metadata_is_rejected(tmp_path: Path) -> None:
    payload = json.loads(EXAMPLE.read_text(encoding="utf-8"))
    payload["firmware_version"] = 7
    invalid = tmp_path / "invalid-firmware.json"
    invalid.write_text(json.dumps(payload), encoding="utf-8")

    with pytest.raises(ScanValidationError, match="firmware_version"):
        load_scan(invalid)
