#!/usr/bin/env bash
# B3 — flash the receive-test firmware and tail logs. Press the real Panasonic remote;
# the log should show decoded frames. Ctrl-C to stop the log stream.
set -eu
HERE="$(cd "$(dirname "$0")" && pwd)"
VENV="${PANAAC_VENV:-/home/hoangminh/AgentsWork/Claude/HA/esphome/.venv}"
# shellcheck disable=SC1091
source "$VENV/bin/activate"
# secrets.yaml (dummy ok for compile; real values only needed to bring the device up on Wi-Fi)
if [ ! -f "$HERE/secrets.yaml" ]; then
  cat > "$HERE/secrets.yaml" <<'EOF'
wifi_ssid: "test-ssid"
wifi_password: "test-password"
EOF
fi
echo ">> compile + upload + log (ensure the ESP is connected via USB)"
esphome run "$HERE/recv-test.yaml"