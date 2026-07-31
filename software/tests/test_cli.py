from pathlib import Path

from radiance3d.cli import main

EXAMPLE = Path(__file__).parents[2] / "data" / "examples" / "simulated" / "dipole-like-scan.json"


def test_validate_command(capsys: object) -> None:
    assert main(["validate", str(EXAMPLE)]) == 0


def test_inspect_command(capsys: object) -> None:
    assert main(["inspect", str(EXAMPLE)]) == 0
