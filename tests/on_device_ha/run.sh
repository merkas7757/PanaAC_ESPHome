#!/usr/bin/env bash
# B5 — flash the HA-integration firmware and tail logs. Then follow the HA verification
# checklist in TEST_ENVIRONMENT.md. Ctrl-C to stop logs.
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
esphome run "$HERE/ha-integration-test.yaml"