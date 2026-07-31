#!/usr/bin/env sh
set -eu

repository_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
environment_dir="$repository_dir/software/.venv"

python3 -c 'import sys; assert sys.version_info >= (3, 11), "Python 3.11+ is required"'
python3 -m venv "$environment_dir"
"$environment_dir/bin/python" -m pip install --upgrade pip
"$environment_dir/bin/python" -m pip install -e "$repository_dir/software[dev]"
"$environment_dir/bin/python" "$repository_dir/scripts/check_repository.py"

printf '%s\n' "Development environment ready at software/.venv"
