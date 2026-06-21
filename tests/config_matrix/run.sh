#!/usr/bin/env bash
# Validate every generated panaac config matrix YAML with `esphome config`.
# No hardware, no toolchain, no secrets required. Reports PASS/FAIL per config.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
VENV="${PANAAC_VENV:-/home/hoangminh/AgentsWork/Claude/HA/esphome/.venv}"
# shellcheck disable=SC1091
source "$VENV/bin/activate"

# Generate fresh configs (idempotent)
python3 "$HERE/generate.py"

configs=( "$HERE"/configs/matrix_*.yaml )
pass=0; fail=0; failed=()
for cfg in "${configs[@]}"; do
  name="$(basename "$cfg")"
  if esphome config "$cfg" >/dev/null 2>"$HERE/.last_err"; then
    echo "PASS  $name"
    pass=$((pass+1))
  else
    echo "FAIL  $name"
    fail=$((fail+1))
    failed+=( "$name" )
    sed 's/^/        /' "$HERE/.last_err" | tail -8
  fi
done
echo "----------------------------------------"
echo "PASS=$pass FAIL=$fail of ${#configs[@]}"
if [ "$fail" -gt 0 ]; then
  echo "Failed configs:"
  printf '  - %s\n' "${failed[@]}"
  exit 1
fi