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
    profile = ROOT / "firmware" / "config" / "provisional-esp32dev-v1.json"
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


def main() -> int:
    errors = [
        *check_required_paths(),
        *check_json(),
        *check_hardware_profile(),
        *check_physical_firmware_has_no_arduino_dependency(),
        *check_internal_links(),
        *check_empty_files(),
    ]
    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        return 1
    print("Repository checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
