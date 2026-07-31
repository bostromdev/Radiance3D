"""Dependency-free domain models for version 1 scan datasets."""

from __future__ import annotations

from collections.abc import Mapping
from dataclasses import dataclass
from datetime import datetime
from math import isfinite
from typing import Any, Literal, cast

DataKind = Literal["measured", "simulated", "imported", "processed"]
TOP_LEVEL_FIELDS = {
    "schema_version",
    "scan_id",
    "scan_name",
    "timestamp",
    "software_version",
    "firmware_version",
    "hardware_configuration",
    "antenna_under_test",
    "rf_source",
    "receiver",
    "frequency_hz",
    "transmit_power",
    "calibration_reference",
    "environmental_notes",
    "warnings",
    "provenance",
    "samples",
}
SAMPLE_FIELDS = {
    "sample_timestamp",
    "azimuth_angle_deg",
    "elevation_angle_deg",
    "measured_value",
    "measurement_unit",
    "quality_flags",
}


def _text(value: object, field: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{field} must be a non-empty string")
    return value


def _string(value: object, field: str) -> str:
    if not isinstance(value, str):
        raise ValueError(f"{field} must be a string")
    return value


def _optional_text(value: object, field: str) -> str | None:
    if value is None:
        return None
    return _text(value, field)


def _number(value: object, field: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{field} must be a number")
    result = float(value)
    if not isfinite(result):
        raise ValueError(f"{field} must be finite")
    return result


def _mapping(value: object, field: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ValueError(f"{field} must be an object")
    return value


def _timestamp(value: object, field: str) -> datetime:
    text = _text(value, field)
    try:
        parsed = datetime.fromisoformat(text.replace("Z", "+00:00"))
    except ValueError as exc:
        raise ValueError(f"{field} must be an ISO 8601 timestamp") from exc
    if parsed.tzinfo is None:
        raise ValueError(f"{field} must include a timezone")
    return parsed


def _string_list(value: object, field: str) -> tuple[str, ...]:
    if not isinstance(value, list) or not all(
        isinstance(item, str) and item.strip() for item in value
    ):
        raise ValueError(f"{field} must be a list of non-empty strings")
    if len(value) != len(set(value)):
        raise ValueError(f"{field} must not contain duplicates")
    return tuple(value)


@dataclass(frozen=True)
class Angle:
    """An angle in degrees with an explicit axis."""

    axis: Literal["azimuth", "elevation"]
    degrees: float

    def __post_init__(self) -> None:
        if not isfinite(self.degrees):
            raise ValueError("angle must be finite")
        lower, upper = (-360.0, 360.0) if self.axis == "azimuth" else (-180.0, 180.0)
        if not lower <= self.degrees <= upper:
            raise ValueError(f"{self.axis} angle must be between {lower:g} and {upper:g} degrees")


@dataclass(frozen=True)
class RFMeasurement:
    """A receiver value and its unit; the value is not assumed to be RSSI."""

    value: float
    unit: str

    def __post_init__(self) -> None:
        if not isfinite(self.value):
            raise ValueError("measurement must be finite")
        if not self.unit.strip():
            raise ValueError("measurement unit must not be empty")

    @classmethod
    def from_mapping(cls, value: object, field: str) -> RFMeasurement:
        item = _mapping(value, field)
        unexpected = set(item) - {"value", "unit"}
        if unexpected:
            raise ValueError(f"{field} contains unsupported fields: {sorted(unexpected)}")
        return cls(
            value=_number(item.get("value"), f"{field}.value"),
            unit=_text(item.get("unit"), f"{field}.unit"),
        )


@dataclass(frozen=True)
class HardwareMetadata:
    """Named, extensible metadata for hardware or a simulator."""

    name: str
    details: Mapping[str, Any]

    @classmethod
    def from_mapping(cls, value: object, field: str) -> HardwareMetadata:
        item = _mapping(value, field)
        return cls(name=_text(item.get("name"), f"{field}.name"), details=dict(item))


@dataclass(frozen=True)
class Sample:
    timestamp: datetime
    azimuth: Angle
    elevation: Angle
    measurement: RFMeasurement
    quality_flags: tuple[str, ...] = ()

    @classmethod
    def from_mapping(cls, value: object, index: int) -> Sample:
        item = _mapping(value, f"samples[{index}]")
        unexpected = set(item) - SAMPLE_FIELDS
        if unexpected:
            raise ValueError(f"samples[{index}] contains unsupported fields: {sorted(unexpected)}")
        return cls(
            timestamp=_timestamp(
                item.get("sample_timestamp"),
                f"samples[{index}].sample_timestamp",
            ),
            azimuth=Angle(
                "azimuth",
                _number(item.get("azimuth_angle_deg"), f"samples[{index}].azimuth_angle_deg"),
            ),
            elevation=Angle(
                "elevation",
                _number(item.get("elevation_angle_deg"), f"samples[{index}].elevation_angle_deg"),
            ),
            measurement=RFMeasurement(
                _number(item.get("measured_value"), f"samples[{index}].measured_value"),
                _text(item.get("measurement_unit"), f"samples[{index}].measurement_unit"),
            ),
            quality_flags=_string_list(
                item.get("quality_flags", []),
                f"samples[{index}].quality_flags",
            ),
        )


@dataclass(frozen=True)
class Scan:
    schema_version: str
    scan_id: str
    name: str
    timestamp: datetime
    software_version: str
    firmware_version: str | None
    frequency_hz: float
    hardware: HardwareMetadata
    antenna_under_test: HardwareMetadata
    rf_source: HardwareMetadata
    receiver: HardwareMetadata
    transmit_power: RFMeasurement | None
    calibration_reference: HardwareMetadata | None
    environmental_notes: str
    data_kind: DataKind
    samples: tuple[Sample, ...]
    warnings: tuple[str, ...]

    @classmethod
    def from_mapping(cls, value: object) -> Scan:
        data = _mapping(value, "scan")
        missing = TOP_LEVEL_FIELDS - set(data)
        if missing:
            raise ValueError(f"missing required fields: {sorted(missing)}")
        unexpected = set(data) - TOP_LEVEL_FIELDS
        if unexpected:
            raise ValueError(f"unsupported top-level fields: {sorted(unexpected)}")

        version = _text(data.get("schema_version"), "schema_version")
        if version != "1.0.0":
            raise ValueError(f"unsupported schema_version: {version}")

        provenance = _mapping(data.get("provenance"), "provenance")
        data_kind_value = provenance.get("data_kind")
        allowed = {"measured", "simulated", "imported", "processed"}
        if not isinstance(data_kind_value, str) or data_kind_value not in allowed:
            raise ValueError(
                "provenance.data_kind must be measured, simulated, imported, or processed"
            )
        data_kind = cast(DataKind, data_kind_value)
        _text(provenance.get("created_by"), "provenance.created_by")
        if "source_dataset_ids" in provenance:
            _string_list(provenance["source_dataset_ids"], "provenance.source_dataset_ids")
        for field in ("method", "notes"):
            if field in provenance:
                _string(provenance[field], f"provenance.{field}")

        samples_value = data.get("samples")
        if not isinstance(samples_value, list) or not samples_value:
            raise ValueError("samples must be a non-empty list")

        frequency_hz = _number(data.get("frequency_hz"), "frequency_hz")
        if frequency_hz <= 0:
            raise ValueError("frequency_hz must be greater than zero")

        transmit_power_value = data.get("transmit_power")
        transmit_power = (
            None
            if transmit_power_value is None
            else RFMeasurement.from_mapping(transmit_power_value, "transmit_power")
        )
        calibration_value = data.get("calibration_reference")
        calibration_reference = (
            None
            if calibration_value is None
            else HardwareMetadata.from_mapping(calibration_value, "calibration_reference")
        )

        return cls(
            schema_version=version,
            scan_id=_text(data.get("scan_id"), "scan_id"),
            name=_text(data.get("scan_name"), "scan_name"),
            timestamp=_timestamp(data.get("timestamp"), "timestamp"),
            software_version=_text(data.get("software_version"), "software_version"),
            firmware_version=_optional_text(data.get("firmware_version"), "firmware_version"),
            frequency_hz=frequency_hz,
            hardware=HardwareMetadata.from_mapping(
                data.get("hardware_configuration"), "hardware_configuration"
            ),
            antenna_under_test=HardwareMetadata.from_mapping(
                data.get("antenna_under_test"), "antenna_under_test"
            ),
            rf_source=HardwareMetadata.from_mapping(data.get("rf_source"), "rf_source"),
            receiver=HardwareMetadata.from_mapping(data.get("receiver"), "receiver"),
            transmit_power=transmit_power,
            calibration_reference=calibration_reference,
            environmental_notes=_string(data.get("environmental_notes"), "environmental_notes"),
            data_kind=data_kind,
            samples=tuple(
                Sample.from_mapping(item, index) for index, item in enumerate(samples_value)
            ),
            warnings=_string_list(data.get("warnings"), "warnings"),
        )
