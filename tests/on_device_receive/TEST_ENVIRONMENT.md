# Scenario B3 — On-device real-remote receive test

**Difficulty:** low (on-device) · **Hardware required:** ESP8266 + IR receiver demodulator +
real Panasonic AC remote · **What it tests:** the IR **receive/decode path** end-to-end on real
hardware — raw timing capture, the `idle: 5ms` two-frame split workaround, byte parsing,
checksum, and the climate/select state update.

## What you need to prepare

1. **ESP8266 board** (e.g. ESP-01 1M, NodeMCU) connected via USB to the computer.
2. **IR receiver demodulator** (38 kHz, e.g. TSOP38238 / VS1838B):
   - VCC → 3.3V (or 5V per the demodulator’s spec; many accept 3.3V)
   - GND → GND
   - DATA → **GPIO14** (matches `recv-test.yaml`)
   - Add a ~100Ω series resistor on VCC and a ~10µF cap across VCC/GND if the demodulator is noisy.
3. **Real Panasonic AC remote** (QKH/TKH/SKH family per the README) and the AC unit nearby
   (you do **not** need the AC unit itself for this test — only the remote).
4. **ESPHome dev venv** at `/home/hoangminh/AgentsWork/Claude/HA/esphome/.venv`.
5. **Wi-Fi credentials** the ESP can join (so you can reach it for logs over API; alternatively
   use `esphome logs` over USB serial). Put real values in `tests/on_device_receive/secrets.yaml`
   (the run script creates a dummy one if missing).

No Home Assistant is required for this scenario.

## How to run

```bash
bash tests/on_device_receive/run.sh
```

This compiles, uploads (over USB), and tails the log at `VERBOSE` level. Then:

1. Point the Panasonic remote at the IR demodulator (a few cm to ~1 m, line of sight).
2. Press buttons: Power, Mode, Temp up/down, Fan, Swing.

## What you should see (pass criteria)

For each remote press, the log should show, e.g.:

```
[V][panaac.climate]: Received raw data size = 308
[V][panaac.climate]: Command decoded: len = 19, data = [ 02 20 E0 04 00 31 30 00 AF 0D ... ]
[V][panaac.climate]: Finish receiving Panasonic AC IR state: len = 19, data = [ ... ]
```

- `raw data size = 308` (second frame only) **or** `440` (full frame) — both are accepted.
- `len = 19` after the fixed 8-byte first frame is cropped.
- The climate entity in Home Assistant (if connected) reflects the remote’s mode/temp/fan/swing.

**Pass:** every remote press produces a `Command decoded` line with `len = 19` and the
climate state updates without errors. **Fail:** repeated `Unexpected data length received` /
`Invalid checksum` / `Decode ir data failed`, or no `Command decoded` lines (wiring/tolerance/
`idle` issue).

## Notes

- `idle: 5ms` is mandatory — the Panasonic signal has a 10 ms inter-frame gap, which equals
  the receiver’s default idle time and makes frame splitting unreliable. See the component README.
- If the demodulator only ever reports `raw data size = 132` (the fixed first frame) and never
  the 308/440 second/full frame, the receiver is missing the second frame — check wiring, 3.3V
  supply stability, and `tolerance: 55%`.
- This test does **not** exercise transmit. Use B1 (loopback) or B2 (optical loopback) for that.