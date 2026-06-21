#!/usr/bin/env bash
# B2 — build + upload both boards. Run twice (once per board) or pass sender|receiver.
#   bash run.sh sender     # flash ESP-A (the IR LED transmitter)
#   bash run.sh receiver   # flash ESP-B (the IR demodulator receiver)
# Then point ESP-A's IR LED at ESP-B's demodulator and press "Start Send Cycle" in HA
# on the sender. Watch the receiver's logs / HA entity for matching state.
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
target="${1:-}"
case "$target" in
  sender)    esphome run "$HERE/sender.yaml" ;;
  receiver)  esphome run "$HERE/receiver.yaml" ;;
  *) echo "usage: $0 sender|receiver"; exit 1 ;;
esac