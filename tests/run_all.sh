#!/usr/bin/env bash
# Run all OFF-DEVICE panaac tests (A1, A3, A2) and report. The on-device scenarios
# (B3/B1/B2/B4/B5) need hardware and are skipped here — see tests/TEST_ENVIRONMENT.md.
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
VENV="${PANAAC_VENV:-/home/hoangminh/AgentsWork/Claude/HA/esphome/.venv}"
# shellcheck disable=SC1091
source "$VENV/bin/activate" 2>/dev/null || { echo "venv not found at $VENV (set PANAAC_VENV)"; exit 1; }

pass=0; fail=0
run() { # run <label> <command...>
  local label="$1"; shift
  echo "==================== $label ===================="
  if "$@"; then echo "RESULT: $label PASS"; pass=$((pass+1)); else echo "RESULT: $label FAIL"; fail=$((fail+1)); fi
}

# A1 — config validation matrix
run "A1-config-matrix" bash "$HERE/config_matrix/run.sh"

# A3 — clang-tidy (needs a compiled panaac build for idedata + clang-tidy installed).
# A3 is an analysis tool, not a pass/fail gate: it exits nonzero when the component has
# findings (expected and documented in FINDINGS.md). "Pass" here = it ran and wrote a report.
if command -v clang-tidy >/dev/null 2>&1 && [ -f "$HERE/../esphome/.esphome/build/ac-test/.pio/build/ac-test/idedata.json" ]; then
  echo "==================== A3-clang-tidy ===================="
  python3 "$HERE/clang_tidy/run_clang_tidy.py" >/tmp/panaac_a3.log 2>&1
  if [ -f "$HERE/clang_tidy/clang_tidy_report.txt" ]; then
    n=$(grep -c "warning:\|error:" "$HERE/clang_tidy/clang_tidy_report.txt" 2>/dev/null || echo 0)
    echo "RESULT: A3-clang-tidy PASS (analysis ran; $n findings — see tests/clang_tidy/FINDINGS.md)"
    pass=$((pass+1))
  else
    echo "RESULT: A3-clang-tidy FAIL (no report produced)"; fail=$((fail+1))
    tail -20 /tmp/panaac_a3.log
  fi
else
  echo "==================== A3-clang-tidy (skipped: needs clang-tidy + a compiled build for idedata) ===================="
fi

# A2 — host protocol unit tests (needs g++)
if command -v g++ >/dev/null 2>&1; then
  run "A2-host-protocol-units" bash "$HERE/host_unit_tests/build.sh"
else
  echo "==================== A2-host-protocol-units (skipped: needs g++) ===================="
fi

echo "========================================"
echo "off-device results: PASS=$pass FAIL=$fail"
echo "on-device scenarios (B3/B1/B2/B4/B5) require hardware — see tests/TEST_ENVIRONMENT.md"
[ "$fail" -eq 0 ]