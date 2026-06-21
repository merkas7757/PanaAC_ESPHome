#!/usr/bin/env bash
# B1 — compile + upload the loopback firmware, then tail logs. After it boots, press the
# "Start Loopback Test" button in Home Assistant (or use `esphome logs` and trigger via API)
# and watch for the LOOPBACK STEP / Command decoded lines. Ctrl-C to stop.
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
VENV="${PANAAC_VENV:-/home/hoangminh/AgentsWork/Claude/HA/esphome/.venv}"
# shellcheck disable=SC1091
source "$VENV/bin/activate"
if [ ! -f "$HERE/secrets.yaml" ]; then
  cat > "$HERE/secrets.yaml" <<'EOF'
wifi_ssid: "test-ssid"
wifi_password: "test-password"
EOF
fi
echo ">> compile + upload + log (ESP connected via USB)"
esphome run "$HERE/loopback-test.yaml"