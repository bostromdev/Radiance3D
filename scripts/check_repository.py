#!/usr/bin/env python3
"""Fast, dependency-free checks for the Radiance3D repository contract."""

from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOFTWARE_SRC = ROOT / "software" / "src"
sys.path.insert(0, str(SOFTWARE_SRC))

from radiance3d.validation import ScanValidationError, load_scan

REQUIRED_PATHS = (
    "README.md",
    "ROADMAP.md",
    "LICENSE",
    "data/schemas/scan-v1.schema.json",
    "data/examples/simulated/dipole-like-scan.json",
    "software/pyproject.toml",
    "firmware/controller/CMakeLists.txt",
    "firmware/controller/main/CMakeLists.txt",
    "firmware/controller/main/Kconfig.projbuild",
    "firmware/controller/main/idf_component.yml",
    "firmware/controller/partitions.csv",
    "firmware/controller/sdkconfig.defaults",
    "firmware/controller/host/CMakeLists.txt",
    "docs/architecture/overview.md",
    "docs/firmware/protocol.md",
    "docs/software/file-formats.md",
)
LINK_PATTERN = re.compile(r"(?<!!)\[[^\]]+\]\(([^)]+)\)")


def check_required_paths() -> list[str]:
    return [
        f"missing required path: {path}"
        for path in REQUIRED_PATHS
        if not (ROOT / path).is_file()
    ]


def check_json() -> list[str]:
    errors: list[str] = []
    for path in ROOT.rglob("*.json"):
        try:
            json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            errors.append(f"{path.relative_to(ROOT)}: invalid JSON: {exc}")

    example = ROOT / "data" / "examples" / "simulated" / "dipole-like-scan.json"
    try:
        scan = load_scan(example)
    except ScanValidationError as exc:
        errors.append(str(exc))
    else:
        if scan.data_kind != "simulated":
            errors.append(f"{example.relative_to(ROOT)}: example must remain simulated")
        if not any("SIMULATED" in warning.upper() for warning in scan.warnings):
            errors.append(f"{example.relative_to(ROOT)}: simulated warning is required")
    return errors


def check_hardware_profile() -> list[str]:
    generator = ROOT / "scripts" / "generate_hardware_profile_header.py"
    profile = ROOT / "firmware" / "config" / "radiance3d-owned-hardware.json"
    try:
        result = subprocess.run(
            [sys.executable, str(generator), "--profile", str(profile), "--validate-only"],
            check=False,
            capture_output=True,
            text=True,
        )
    except OSError as exc:
        return [f"could not validate hardware profile: {exc}"]
    if result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or "unknown error"
        return [f"hardware profile validation failed: {detail}"]
    return []


def check_physical_firmware_has_no_arduino_dependency() -> list[str]:
    forbidden = (
        "#include <Arduino.h>",
        "HardwareSerial",
        "pinMode(",
        "digitalWrite(",
        "digitalRead(",
        "void setup(",
        "void loop(",
        "Ticker.h",
        "ESP32TimerInterrupt",
        "TMCStepper",
        "AccelStepper",
        "ArduinoJson",
        "framework = arduino",
    )
    errors: list[str] = []
    firmware = ROOT / "firmware" / "controller"
    for path in firmware.rglob("*"):
        if not path.is_file() or (
            path.suffix not in {".cpp", ".hpp", ".h", ".ini", ".cmake"}
            and path.name != "CMakeLists.txt"
        ):
            continue
        text = path.read_text(encoding="utf-8")
        for marker in forbidden:
            if marker in text:
                errors.append(f"{path.relative_to(ROOT)}: Arduino dependency {marker!r}")
        for api in ("delay", "millis", "micros"):
            if re.search(rf"\b{api}\s*\(", text):
                errors.append(
                    f"{path.relative_to(ROOT)}: Arduino dependency {api + '()'!r}"
                )
    return errors


def check_internal_links() -> list[str]:
    errors: list[str] = []
    for markdown in ROOT.rglob("*.md"):
        text = markdown.read_text(encoding="utf-8")
        for target in LINK_PATTERN.findall(text):
            target = target.strip().strip("<>")
            if target.startswith(("http://", "https://", "mailto:", "#")):
                continue
            path_text = target.split("#", 1)[0]
            if not path_text:
                continue
            resolved = (markdown.parent / path_text).resolve()
            if not resolved.exists():
                errors.append(
                    f"{markdown.relative_to(ROOT)}: broken internal link target {target!r}"
                )
    return errors


def check_empty_files() -> list[str]:
    allowed = {ROOT / "software" / "src" / "radiance3d" / "py.typed"}
    errors: list[str] = []
    for path in ROOT.rglob("*"):
        # ESP-IDF creates empty generated stamp/source placeholders under its
        # ignored build directory. They are not repository artifacts.
        if "build" in path.relative_to(ROOT).parts:
            continue
        if path.is_file() and path.stat().st_size == 0 and path not in allowed:
            errors.append(f"{path.relative_to(ROOT)}: empty file")
    return errors


def check_owned_hardware_contract() -> list[str]:
    """Reject production documentation/configuration that drifts from owned parts."""
    errors: list[str] = []
    forbidden = re.compile(r"\b(A4988|DRV8825|TMC2208|AD8318|RX5808|Raspberry\s*Pi|Arduino|slip\s*ring)\b", re.I)
    for path in ROOT.rglob("*"):
        if not path.is_file() or path.suffix not in {".md", ".json", ".cpp", ".hpp", ".py"}:
            continue
        if "Measurements" in path.parts or path == Path(__file__):
            continue
        text = path.read_text(encoding="utf-8")
        # Future-expansion examples are explicitly non-production and may mention
        # hardware that is intentionally excluded from Revision 1.
        if path == ROOT / "docs" / "hardware" / "reference-architecture.md":
            text = text.split("## Future Expansion", 1)[0]
        match = forbidden.search(text)
        if match:
            errors.append(f"{path.relative_to(ROOT)}: unsupported production hardware {match.group(0)!r}")
    profile_path = ROOT / "firmware/config/radiance3d-owned-hardware.json"
    try:
        profile = json.loads(profile_path.read_text(encoding="utf-8"))
        power = profile["power"]
        detector = profile["detector"]
        axes = profile["axes"]
    except (OSError, KeyError, TypeError, json.JSONDecodeError) as exc:
        return [f"owned hardware profile cannot be checked: {exc}"]
    if power.get("outputs_paralleled") is not False:
        errors.append("owned hardware profile: buck outputs are paralleled")
    external_input = power.get("external_input", {})
    if external_input.get("conductors") != ["+12V IN", "GND IN"]:
        errors.append("owned hardware profile: external input must be +12V IN / GND IN")
    if "strain-relieved" not in external_input.get("assembly_entry_requirement", ""):
        errors.append("owned hardware profile: external input requires strain relief")
    architecture = profile.get("mechanical_architecture", {})
    if architecture.get("enclosure_designed") is not False:
        errors.append("owned hardware profile: enclosure must remain undesigned")
    if "one controlled 360 degree rotation" not in architecture.get("pan_rotation", ""):
        errors.append("owned hardware profile: pan must specify one controlled 360 degree rotation")
    if "tilt motor" not in architecture.get("rotating_platform", []) or "AD8317 detector" not in architecture.get("rotating_platform", []):
        errors.append("owned hardware profile: rotating platform must contain tilt motor and AD8317")
    if "no RG316 jumper" not in architecture.get("rf_measurement", ""):
        errors.append("owned hardware profile: direct-SMA detector mounting is required")
    if power.get("digital_control_voltage_v") != 5.0:
        errors.append("owned hardware profile: ESP32 branch must be regulated 5.0 V")
    if power.get("rf_analog_voltage_v") != 5.0:
        errors.append("owned hardware profile: AD8317 branch must be regulated 5.0 V")
    if detector.get("adc_unit") != "ADC1":
        errors.append("owned hardware profile: detector must use ADC1")
    for name, axis in axes.items():
        if axis.get("maximum_rms_current_ma", 0) > 800:
            errors.append(f"owned hardware profile: {name} current exceeds 800 mA RMS ceiling")
        if axis.get("home_switch_pin") != -1:
            errors.append(f"owned hardware profile: {name} assumes an unowned home switch")
    harness = (ROOT / "docs/hardware/wire-standard.md").read_text(encoding="utf-8")
    for required in ("18 AWG", "22 AWG", "26 AWG", "PWR-000", "PWR-001", "SIG-001", "MTR-001",
                     "A1/A+ → Black", "A2/A− → Yellow", "B1/B+ → Red", "B2/B− → Blue",
                     "labels at both ends"):
        if required not in harness:
            errors.append(f"harness contract missing {required!r}")
    for required in ("RG316 50 Ω coax", "RF-001", "centre conductor", "shield",
                     "no baseline\nRG316 jumper"):
        if required not in harness:
            errors.append(f"RF cabling contract missing {required!r}")
    if "| RF-002 |" in harness:
        errors.append("RF cabling contract contains prohibited antenna-to-detector jumper")
    if "H-" in harness:
        errors.append("harness contract contains retired mixed-purpose H- identifier")
    routing = (ROOT / "docs/hardware/assembly-order.md").read_text(encoding="utf-8")
    if "Unlimited continuous pan rotation is not allowed." not in routing:
        errors.append("cable-routing contract permits unlimited pan rotation")
    if "one controlled 360° turn" not in routing:
        errors.append("cable-routing contract lacks controlled 360 degree pan rotation")
    power_tree = (ROOT / "docs/hardware/power-tree.md").read_text(encoding="utf-8")
    for required in ("Off-board 12 V bench power source", "+12V IN", "GND IN", "no battery compartment"):
        if required not in power_tree:
            errors.append(f"external-power contract missing {required!r}")
    reference = (ROOT / "docs/hardware/reference-architecture.md").read_text(encoding="utf-8")
    for required in ("DESIGN INTENT: STATIONARY BASE", "DESIGN INTENT: ROTATING PLATFORM",
                     "external stationary 5.8 GHz VTX", "AUT directly threads onto the AD8317 SMA",
                     "no detector-to-antenna RG316 jumper", "Conceptual architecture only.",
                     "Final placement determined during CAD.",
                     "Firmware-defined return-to-home strategy to prevent cumulative cable twist"):
        if required not in reference:
            errors.append(f"reference architecture missing {required!r}")
    return errors


def main() -> int:
    errors = [
        *check_required_paths(),
        *check_json(),
        *check_hardware_profile(),
        *check_physical_firmware_has_no_arduino_dependency(),
        *check_internal_links(),
        *check_empty_files(),
        *check_owned_hardware_contract(),
    ]
    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        return 1
    print("Repository checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
