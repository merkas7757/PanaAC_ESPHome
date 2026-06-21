#!/usr/bin/env bash
# B4 — flash the real-AC test firmware and tail logs. Then press "Run AC Test" in HA and
# watch the AC physically respond to each step. Ctrl-C to stop logs.
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
esphome run "$HERE/real-ac-test.yaml"