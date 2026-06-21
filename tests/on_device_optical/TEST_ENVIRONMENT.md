# Scenario B2 — On-device two-board optical loopback

**Difficulty:** medium (on-device) · **Hardware required:** two ESP8266 boards, one IR LED
(+ transistor driver), one IR demodulator, Home Assistant · **What it tests:** the full
encode → 38 kHz optical IR → decode path with real IR components — the path that B1's
electrical self-loopback and the off-device tests cannot cover.

## What you need to prepare

1. **ESP-A (sender)** + an **IR LED driven through a transistor** (a GPIO can't source enough
   current for an IR LED directly; use an NPN transistor + ~10–100Ω resistor). LED anode to
   +3.3/+5V via resistor, cathode to transistor collector, emitter to GND, base via ~1kΩ to
   **GPIO13**.
2. **ESP-B (receiver)** + a **38 kHz IR demodulator** (TSOP38238/VS1838B): DATA → **GPIO14**,
   VCC → 3.3V, GND → GND (decoupling cap recommended).
3. **Home Assistant** on the same network (both boards join Wi-Fi via `secrets.yaml` and
   expose the API; HA auto-discovers the climate + selects on each).
4. Place ESP-A's IR LED a few cm–1 m from ESP-B's demodulator, line of sight.

## How to run

```bash
bash tests/on_device_optical/run.sh sender     # flash + log ESP-A
bash tests/on_device_optical/run.sh receiver   # flash + log ESP-B
```

Then in Home Assistant, on the **sender** device, press **Start Send Cycle**.

## What you should see (pass criteria)

On the **receiver** logs, for each of the 3 sender steps:

```
[V][panaac.climate]: Command decoded: len = 19, data = [ ... ]
```

and the receiver's `PanaAC Receiver` climate entity in HA should match each commanded state:

| Step | Commanded (sender) | Expected receiver state |
|------|--------------------|------------------------|
| 1 | COOL 24°C, fan LOW, swing VERTICAL | mode=COOL, temp=24.0, fan=LOW, swing=VERTICAL |
| 2 | HEAT 26.5°C, fan MEDIUM, swing OFF | mode=HEAT, temp=26.5, fan=MEDIUM, swing=OFF |
| 3 | DRY 22°C, fan AUTO, swing BOTH | mode=DRY, temp=22.0, fan=AUTO, swing=BOTH |

**Pass:** every sender step produces a matching `Command decoded` on the receiver and the
receiver's HA entity mirrors the sender's commanded state within ~2 s.

**Fail:** no `Command decoded` (check IR LED orientation, transistor wiring, 38 kHz carrier
`ir_control: true`, demodulator supply/decoupling, line-of-sight/distance), or decoded state
differs from commanded (encode/decode mismatch — investigate with the A2 host tests).

## Notes

- Both configs use `ir_control: true` (38 kHz carrier) — required for real IR LEDs, unlike B1.
- This validates the non-invasive “method 2” from the component README, which is what most users
  will actually deploy.
- Two boards are needed because a single board cannot optically loop back to itself (the
  demodulator cannot see its own LED at DC; optical isolation requires two devices).