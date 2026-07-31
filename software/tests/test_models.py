import json
from pathlib import Path

import pytest

from radiance3d.models import Angle
from radiance3d.validation import ScanValidationError, load_scan

EXAMPLE = Path(__file__).parents[2] / "data" / "examples" / "simulated" / "dipole-like-scan.json"


def test_simulated_example_loads() -> None:
    scan = load_scan(EXAMPLE)
    assert scan.schema_version == "1.1.0"
    assert scan.data_kind == "simulated"
    assert len(scan.samples) == 5
    assert scan.samples[0].measurement.unit == "dB_relative"
    assert scan.samples[0].measurement_source == "deterministic-pattern-generator"
    assert scan.samples[0].position_kind == "commanded"
    assert scan.samples[0].sequence_number == 0
    assert scan.protocol_version == "1"
    assert scan.hardware_revision == "simulator"
    assert scan.step_size is not None
    assert scan.step_size.azimuth_deg == 45
    assert scan.measurement_units == ("dB_relative",)
    assert scan.calibration_status == "not-applicable"


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


def test_legacy_1_0_scan_remains_readable(tmp_path: Path) -> None:
    payload = json.loads(EXAMPLE.read_text(encoding="utf-8"))
    payload["schema_version"] = "1.0.0"
    for field in (
        "protocol_version",
        "hardware_revision",
        "step_size_deg",
        "measurement_units",
        "calibration_status",
        "operator_notes",
    ):
        del payload[field]
    for sample in payload["samples"]:
        for field in (
            "sequence_number",
            "measurement_source",
            "position_kind",
            "validity",
            "warnings",
        ):
            del sample[field]
    legacy = tmp_path / "legacy.json"
    legacy.write_text(json.dumps(payload), encoding="utf-8")

    scan = load_scan(legacy)

    assert scan.schema_version == "1.0.0"
    assert scan.protocol_version is None
    assert scan.samples[0].sequence_number is None


def test_new_scan_requires_contiguous_sequence_numbers(tmp_path: Path) -> None:
    payload = json.loads(EXAMPLE.read_text(encoding="utf-8"))
    payload["samples"][2]["sequence_number"] = 7
    invalid = tmp_path / "invalid-sequence.json"
    invalid.write_text(json.dumps(payload), encoding="utf-8")

    with pytest.raises(ScanValidationError, match="contiguous from zero"):
        load_scan(invalid)


def test_declared_measurement_units_must_match_samples(tmp_path: Path) -> None:
    payload = json.loads(EXAMPLE.read_text(encoding="utf-8"))
    payload["measurement_units"] = ["dBm"]
    invalid = tmp_path / "invalid-units.json"
    invalid.write_text(json.dumps(payload), encoding="utf-8")

    with pytest.raises(ScanValidationError, match="must exactly match"):
        load_scan(invalid)


def test_calibrated_scan_requires_reference_metadata(tmp_path: Path) -> None:
    payload = json.loads(EXAMPLE.read_text(encoding="utf-8"))
    payload["calibration_status"] = "calibrated"
    invalid = tmp_path / "invalid-calibration.json"
    invalid.write_text(json.dumps(payload), encoding="utf-8")

    with pytest.raises(ScanValidationError, match="require calibration_reference"):
        load_scan(invalid)


def test_processed_scan_requires_source_ids_and_method(tmp_path: Path) -> None:
    payload = json.loads(EXAMPLE.read_text(encoding="utf-8"))
    payload["provenance"]["data_kind"] = "processed"
    del payload["provenance"]["method"]
    invalid = tmp_path / "invalid-provenance.json"
    invalid.write_text(json.dumps(payload), encoding="utf-8")

    with pytest.raises(ScanValidationError, match="source_dataset_ids"):
        load_scan(invalid)
