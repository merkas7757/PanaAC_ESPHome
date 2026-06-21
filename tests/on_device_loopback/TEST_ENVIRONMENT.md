# Scenario B1 — On-device single-board loopback round-trip

**Difficulty:** medium (on-device) · **Hardware required:** one ESP8266, no IR LEDs, no AC ·
**What it tests:** the full **encode → transmit → receive → decode** round-trip on real
hardware, including IR timing and the `idle: 5ms` two-frame split — something no off-device
test can verify. This is the strongest on-device functional test.

## Why it works with one board

Invasive mode uses the **same GPIO4 for receiver and transmitter** (`allow_other_uses: true`,
`OUTPUT_OPEN_DRAIN`). With `ir_control: false` the transmitter drives the pin **baseband** (no
38 kHz carrier), and the receiver reads the same pin — so the board **electrically receives
its own transmission** with no second device and no optical alignment. The round-trip is exact.

(For non-invasive / real-IR-LED mode you cannot self-loopback optically — use B2 with two
boards.)

## What you need to prepare

1. **ESP8266 board** connected via USB.
2. **GPIO4** left as the shared IR pin — **nothing else wired** (no external IR LED, no
   demodulator; the loopback is internal to the pin). If you are also wired to an AC IR board,
   disconnect it for this test so the AC isn’t driven.
3. **ESPHome dev venv** + Wi-Fi credentials in `secrets.yaml` (run script creates a dummy one).
4. **Home Assistant** (or the ESPHome API) to press the “Start Loopback Test” button. If you
   have no HA, you can still observe boot/decode logs over USB with `esphome logs`, but
   triggering the scripted cycle needs the button (HA) or an API call.

## How to run

```bash
bash tests/on_device_loopback/run.sh        # compile + upload + tail logs
```

Then press **Start Loopback Test** in Home Assistant (or call the button via the API).

## What you should see (pass criteria)

For each of the 3 scripted steps you should see, in order:

1. `LOOPBACK STEP N: <commanded state>`
2. `[V][panaac.climate]: Command decoded: len = 19, data = [ ... ]`  ← the self-received frame
3. `after step N: mode=<m> temp=<t> fan_mode=<f> swing=<s>`  ← the resulting climate state

**Pass:** every step shows a `Command decoded` line, and the `after step N` state **matches**
the commanded state (e.g. step 1 commands COOL/24/LOW/VERTICAL → after step 1 reports
`mode=2 temp=24.0 fan_mode=3 swing=2`). Stable state = encode→decode is consistent on hardware.

**Fail / drift:** the `after step N` state differs from the commanded state (decode(encode(X))
≠ X), or no `Command decoded` line appears (self-receive not working — check `allow_other_uses`,
`idle: 5ms`, `ir_control: false`).

## Notes

- The numeric codes: mode OFF=0 HEAT_COOL=1 COOL=2 HEAT=3 FAN_ONLY=4 DRY=5 AUTO=6;
  fan AUTO=2 LOW=3 MEDIUM=4 HIGH=5 QUIET=9; swing OFF=0 BOTH=1 VERTICAL=2 HORIZONTAL=3.
- This test exercises the **climate `transmit_state` path** (via `climate.control`), which is
  the path that loses the fan L2/L4 granularity (see A2 `transmit_state_loses_level2`). The
  script uses only fan_mode values (LOW/MEDIUM/AUTO), not L2/L4, so it stays within what the
  climate path can represent.
- To additionally test the select (L2/L4) path on hardware, drive the `fanlevel`/`swingv`/
  `swingh` selects from HA and confirm each produces a matching `Command decoded` line.