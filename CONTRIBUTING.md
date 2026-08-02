# Contributing to Radiance3D

Radiance3D is in architecture and prototyping. Contributions should improve a
documented interface, testable behavior, or an evidence-backed design decision.

## Before starting

1. Search existing issues and discussions.
2. Open an issue before large hardware, protocol, schema, or dependency changes.
3. Distinguish measured, simulated, imported, and processed data.
4. Do not claim hardware compatibility or accuracy without reproducible evidence.

## Development workflow

1. Branch from `main` using a descriptive name.
2. Keep commits focused and avoid generated artifacts.
3. Run `python scripts/check_repository.py`.
4. For software changes, run Ruff, mypy, and pytest from `software/`.
5. For firmware changes, run the host CMake/CTest suite and `idf.py build` when
   ESP-IDF is available.
6. Update relevant documentation and the changelog when behavior changes.

Pull requests should explain the problem, the chosen design, validation performed,
and any remaining uncertainty. Hardware changes should include units, constraints,
safety considerations, and the revision of any referenced component.

## Engineering records

Record consequential decisions in `docs/architecture/design-decisions.md`.
Experimental results should include raw provenance, configuration, timestamps,
and limitations. Never replace raw data with processed output.
