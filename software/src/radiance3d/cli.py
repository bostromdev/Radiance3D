"""Command-line interface for honest, local scan-file operations."""

from __future__ import annotations

import argparse
from collections.abc import Sequence

from radiance3d.validation import ScanValidationError, load_scan


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="radiance3d")
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("validate", "inspect"):
        command = commands.add_parser(name)
        command.add_argument("path", help="Path to a Radiance3D JSON scan")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    args = _parser().parse_args(argv)
    try:
        scan = load_scan(args.path)
    except ScanValidationError as exc:
        print(f"error: {exc}")
        return 1

    if args.command == "validate":
        print(f"valid Radiance3D {scan.schema_version} scan: {scan.scan_id}")
    else:
        units = sorted({sample.measurement.unit for sample in scan.samples})
        print(f"name: {scan.name}")
        print(f"id: {scan.scan_id}")
        print(f"kind: {scan.data_kind}")
        print(f"frequency_hz: {scan.frequency_hz:g}")
        print(f"samples: {len(scan.samples)}")
        print(f"measurement_units: {', '.join(units)}")
        print(f"warnings: {len(scan.warnings)}")
    return 0
