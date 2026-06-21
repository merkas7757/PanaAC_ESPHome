# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

`PanaAC_ESPHome` is an **ESPHome external custom component** that controls Panasonic air
conditioners via infrared. It is not part of the ESPHome firmware toolchain itself — it is a
drop-in component that users install into their own ESPHome `components/` folder (or load via
`external_components`). The ESPHome platform source it targets lives in the sibling
`../esphome/` directory; follow `../esphome/CLAUDE.md` for the platform's coding conventions
(C++ style, codegen patterns, embedded heap-allocation rules, component structure). This file
only documents what is specific to this component.

The component exposes one `climate` platform (`panaac`) plus three companion `select` entities
(fan level, vertical swing, horizontal swing).

## Repository layout

```
esphome/
├── ac-test-1.yaml          # Example config — invasive wiring (receiver+transmitter share GPIO4)
├── ac-test-2.yaml          # Example config — non-invasive (separate IR LEDs, ir_control: true)
└── components/panaac/
    ├── __init__.py          # empty (required so ESPHome loads the component)
    ├── climate.py           # CONFIG_SCHEMA + to_code (codegen) — the platform entrypoint
    ├── definitions.h        # IR timing constants, byte positions, enums (FanLevel/SwingVPos/SwingHPos), ClimateState struct, option strings
    ├── panaac.h / panaac.cpp        # PanaACClimate : climate_ir::ClimateIR — setup/traits/transmit/on_receive
    └── extra.h / extra.cpp          # PanaACFanLevel / PanaACSwingV / PanaACSwingH : select::Select
```

## Protocol model

Panasonic AC IR is a **two-frame, 27-byte (216-bit) signal** with a 10 ms inter-frame gap:

- **Frame 1** — 8 fixed bytes: `0x02 0x20 0xE0 0x04 0x00 0x00 0x00 0x06` (a "magic" header).
- **Frame 2** — 19 bytes of AC state. Bytes 0–4 are `0x02 0x20 0xE0 0x04 0x00`; byte 18 is an
  additive checksum of bytes 0–17.

The decoder accepts either the full 27-byte capture (it crops the first 8 bytes) or a lone
19-byte second frame, because ESPHome's `remote_receiver` with the default 10 ms `idle` time
unreliably splits the two frames. The README documents the workaround: set `idle: 5ms` on the
`remote_receiver` so the signal always arrives as two separate frames.

Bit encoding: header mark 3650 µs / space 1600 µs; bit mark 550 µs; one-space 1200 µs;
zero-space 350 µs; frame end 10000 µs. LSB-first within each byte. Carrier 38 kHz (only when
driving a real IR LED — see `ir_control`).

State byte layout (`definitions.h` — `PANAAC_BYTEPOS_*`): power+mode share byte 5 (bit 0 =
power, upper nibble = mode), temperature byte 6, fan+swingV byte 8 (upper nibble fan, lower
nibble vertical swing), swingH byte 9, quiet byte 13.

## Configuration keys (climate.py)

Beyond the standard `climate_ir` keys (`supports_cool`, `supports_heat`, `sensor`,
`humidity_sensor`, `receiver_id`), the schema adds:

| Key | Default | Effect |
|-----|---------|--------|
| `supports_fan_only` | false | Advertise `CLIMATE_MODE_FAN_ONLY` |
| `supports_quiet` | false | Advertise `CLIMATE_FAN_QUIET`; add a "Quiet" select option |
| `fan_5level` | false | 5 fan levels (L1–L5) in the select vs default 3 (L1/L3/L5) |
| `swing_horizontal` | false | Enable horizontal swing + the SwingH select + `SWING_HORIZONTAL`/`SWING_BOTH` modes |
| `temp_step` | 1.0 | Visual temperature step (0.5 or 1.0); enables half-degree encoding |
| `ir_control` | false | **false** = invasive direct-wired (no 38 kHz carrier). **true** = non-invasive real IR LED (set 38 kHz carrier) |

`ir_control` must be `true` when driving physical IR LEDs (separate receiver/transmitter
GPIOs) and `false` when wired directly to the AC's IR board (shared GPIO, `OUTPUT_OPEN_DRAIN`,
`allow_other_uses: true`). See `ac-test-1.yaml` vs `ac-test-2.yaml`.

## Architecture notes that matter when editing

- **`PanaACClimate` inherits `climate_ir::ClimateIR`** (in `../esphome/.../climate_ir/`).
  `ClimateIR` supplies `control()` (applies a `ClimateCall` then calls `transmit_state()`),
  `setup()` (restores saved state from flash), `dump_config()`, and the receiver/transmitter
  wiring. PanaAC overrides `setup()`, `traits()`, `transmit_state()`, `on_receive()`.
- **Two transmit code paths exist and can diverge:**
  - `transmit_state()` — called by the inherited `ClimateIR::control()` when the user drives
    the climate entity. It *re-derives* `ac_state` from `this->mode/target_temperature/fan_mode/
    swing_mode`, then calls `transmit_data()`.
  - `update_state()` — called by the three `Select::control()` overrides. It transmits
    `ac_state` *as-is* (the select already mutated the relevant field), then re-publishes.
  Both call `transmit_data()` (the raw IR encoder) and `publish_state()`. Keep them in sync;
  see "Known issues" in the platform review for where they already disagree.
- **The selects are created in `climate.py` `to_code`**, not from YAML. Options are *not* passed
  to `select.register_select` (empty list); they are filled at runtime in `PanaACClimate::setup()`
  via `traits.set_options(...)` based on the `fan_5level` / `supports_quiet` / `swing_horizontal`
  flags. The select IDs (`swingv_id`, `swingh_id`, `fanlevel_id`) are auto-generated.
- **`ac_state` is a public field** on `PanaACClimate` so the select classes can read/write it
  directly (`extra.cpp` does `this->climate_->ac_state.fan_mode = ...`).
- **Fan-mode ↔ fan-level mapping** is lossy at the climate level: ESPHome `ClimateFanMode` only
  has AUTO/LOW/MEDIUM/HIGH/QUIET, so levels 2 and 4 are reachable only through the `select`,
  not through `fan_mode`. `transmit_state()` collapses LOW→L1, MEDIUM→L3, HIGH→L5; `transmit_data()`
  preserves an existing L2/L4 if `fan_mode` still matches.

## Common tasks

- **Validate an example config against the platform** (from the workspace root, requires the
  ESPHome toolchain installed): `esphome config PanaAC_ESPHome/esphome/ac-test-1.yaml`
- **Compile without flashing:** `esphome compile PanaAC_ESPHome/esphome/ac-test-1.yaml`
- **Lint C++** per platform rules: run `clang-format` on the `.h`/`.cpp` files (the component
  currently uses 4-space indent; the platform uses 2-space — see review).
- The example YAMLs use `external_components: - source: components` with secrets
  (`!secret wifi_ssid` etc.); supply a `secrets.yaml` or substitute real values before compiling.

## License

Apache 2.0 (headers in each source file). Maintenance is voluntary; the README links to a
"Buy Me A Coffee" page. Tested by the author against Panasonic QKH/TKH/SKH units with ESP8266
and ESP32.