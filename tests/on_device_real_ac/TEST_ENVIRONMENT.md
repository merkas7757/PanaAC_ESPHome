# Scenario B4 — On-device end-to-end against the real Panasonic AC

**Difficulty:** low-effort code, but needs the physical AC · **Hardware required:** ESP8266
wired (invasively) to the Panasonic AC's IR board **or** an IR LED pointed at the AC, plus the
AC itself · **What it tests:** the ultimate end-to-end behavior — does the encoded IR command
actually make the real AC do the right thing? No off-device or loopback test can answer this.

## What you need to prepare

1. **A Panasonic AC** with its IR remote receiver working (QKH/TKH/SKH family per the README).
2. **ESP8266 wired to the AC** — the invasive “method 1” from the component README:
   - ESP GND ↔ AC IR board GND
   - ESP 5V ↔ AC IR board VCC (confirm the board supplies 5V)
   - ESP **GPIO4** ↔ AC IR board IR-LED signal pin
   - This config uses the shared-pin invasive mode (`ir_control: false`, baseband).
   
   **Alternative:** the non-invasive “method 2” — an IR LED (with transistor driver) on GPIO4
   pointed at the AC, with `ir_control: true`. Use the B2 receiver config as a template and
   set `ir_control: true`.
3. **Home Assistant** to press the “Run AC Test” button (or trigger via API).
4. ESPHome dev venv + Wi-Fi credentials in `secrets.yaml`.

## How to run

```bash
bash tests/on_device_real_ac/run.sh        # compile + upload + log
```

Then in Home Assistant press **Run AC Test** and **watch the AC** (beeps/display/louvers/airflow).

## What you should observe (pass criteria)

The script runs 5 steps; confirm the AC responds to each:

| Step | Command | Expected AC behavior |
|------|---------|----------------------|
| 1 | Power ON, COOL 24°C, fan AUTO, swing VERTICAL | AC turns on, beeps, cool mode, 24°C, vertical louvers sweeping |
| 2 | → 27°C | AC display/behavior updates to 27°C |
| 3 | fan HIGH | Fan speed increases |
| 4 | swing OFF | Louvers stop at a fixed position |
| 5 | Power OFF | AC turns off |

**Pass:** the AC visibly responds to every step with the correct mode/temp/fan/swing.

**Fail / partial:** the AC ignores commands (check invasive wiring — 5V/GND/signal, board
supplies 5V not 3.3V — or for non-invasive, IR LED orientation/distance/`ir_control: true`),
or responds to some commands but not others (likely an encode mismatch for that specific
field — narrow down with the A2 host tests and the B1 loopback test).

## Notes

- The AC gives **no feedback** to the ESP, so this test is inherently **manual/observational** —
  a human must watch the AC. There is no automated pass/fail signal.
- If the AC also emits IR (its remote does), the shared-pin receiver on GPIO4 will capture
  real-remote presses too — useful to cross-check the ESP’s view against the physical remote.
- This is the only scenario that validates the actual Panasonic protocol against real hardware,
  so it’s the final acceptance test before considering the component “verified.”