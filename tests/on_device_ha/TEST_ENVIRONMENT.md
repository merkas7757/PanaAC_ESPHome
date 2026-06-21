# Scenario B5 — On-device Home Assistant entity integration

**Difficulty:** low (on-device) · **Hardware required:** one ESP8266 (invasive shared-GPIO4,
no external IR parts needed) + Home Assistant · **What it tests:** the **entity surface** the
component exposes to HA — that the climate card and the three `select` entities are discovered,
that their options reflect the config flags (`fan_5level`, `supports_quiet`,
`swing_horizontal`), and that state flows both directions (HA → ESP transmits; ESP receive →
HA updates).

## What you need to prepare

1. **ESP8266** connected via USB. No IR LED/demodulator/AC required — this test uses the
   invasive shared-GPIO4 self-loopback so the ESP receives its own transmit (like B1). Leave
   GPIO4 otherwise unconnected.
2. **Home Assistant** on the same network, with the ESPHome integration installed (it usually
   is by default). The board joins Wi-Fi via `secrets.yaml` and advertises via mDNS; HA will
   auto-discover it.
3. ESPHome dev venv + Wi-Fi credentials in `secrets.yaml`.

## How to run

```bash
bash tests/on_device_ha/run.sh        # compile + upload + log
```

In Home Assistant: **Settings → Devices & Services → ESPHome**, adopt the new device
(`PanaAC HA Integration`), then open its device page.

## Verification checklist (pass criteria)

With all flags on (`fan_5level`, `supports_quiet`, `swing_horizontal` all true), confirm:

- [ ] **Climate entity** `PanaAC HA` appears with a climate card.
- [ ] **Supported modes** shown: OFF, AUTO, COOL, HEAT, DRY, FAN_ONLY (cool/heat/fan_only all
      enabled by the config).
- [ ] **Temperature** slider: min 16°C, max 30°C, **0.5°C step** (from `temp_step: 0.5`).
- [ ] **Fan modes** on the climate: AUTO, LOW, MEDIUM, HIGH, QUIET.
- [ ] **Swing modes** on the climate: OFF, VERTICAL, HORIZONTAL, BOTH.
- [ ] **Select “Fan Level”** appears with **7 options**: Auto, Level 1, Level 2, Level 3,
      Level 4, Level 5, Quiet.
- [ ] **Select “Swing Vertical”** appears with 6 options: Auto, Highest, High, Middle, Low,
      Lowest.
- [ ] **Select “Swing Horizontal”** appears with 6 options: Auto, Left Max, Left, Middle,
      Right, Right Max.
- [ ] **HA → ESP:** change the climate mode/temp/fan/swing in HA; the ESP log shows the
      transmit (`Sending Panasonic AC IR state: ...`) and (self-loopback) a matching
      `Command decoded` line; the HA state holds steady (no flicker).
- [ ] **Select → ESP:** pick “Level 2” in the Fan Level select; the ESP log shows it; the
      select state holds at “Level 2”.
- [ ] **ESP → HA:** if you have a real remote + receiver wired, pressing the remote updates
      the HA climate + selects (cross-check with B3). With only self-loopback, the ESP→HA
      path is exercised by the self-receive after each HA command.

**Pass:** every checkbox above holds. **Fail:** a missing entity, wrong option list (e.g.
only 3 fan levels when `fan_5level: true`), wrong temp step, or HA state flickering after a
command (round-trip mismatch — investigate with A2/B1).

## Notes

- To verify the option lists **shrink** correctly, re-flash with flags off and re-check:
  `fan_5level: false` → Fan Level has Auto, Level 1, Level 3, Level 5 (4 options);
  `supports_quiet: false` → no Quiet option; `swing_horizontal: false` → no Swing Horizontal
  select and climate swing modes only OFF/VERTICAL.
- This scenario focuses on the **integration/entity model**, not IR correctness — pair it
  with B1/B2/B4 for end-to-end IR validation.