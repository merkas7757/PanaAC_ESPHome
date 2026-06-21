# Scenario A1 — Off-device config validation matrix

**Difficulty:** easiest · **Hardware required:** none · **What it tests:** the panaac
`CONFIG_SCHEMA` and `to_code()` (select creation + options) across every meaningful
combination of the component's boolean flags + `temp_step`.

## What you need to prepare

This scenario is **fully off-device** — no ESP board, no IR hardware, no Home Assistant,
no secrets, no PlatformIO toolchain download. You only need the workspace ESPHome dev venv.

1. **ESPHome dev venv** at `/home/hoangminh/AgentsWork/Claude/HA/esphome/.venv`
   (Python 3.13, esphome installed editable). See workspace `DEV_ENVIRONMENT.md` if it is
   missing. Nothing else to install.
2. **The component source** at `esphome/components/panaac/` (loaded via
   `external_components: source: ../../../esphome/components` in each generated config).

No network is required after the venv exists (esphome is already installed; `esphome config`
does not download toolchains).

## Files

- `generate.py` — emits 12 curated config YAMLs into `configs/` covering: all-defaults,
  `fan_5level`, `supports_quiet`, 5-level+quiet (7 fan options), `swing_horizontal`,
  `temp_step: 0.5`, `ir_control: true`, `fan_only`+heat with cool off, everything-on,
  cool-only minimal, and combined edge cases.
- `run.sh` — activates the venv, (re)generates the configs, runs `esphome config` on each,
  prints `PASS`/`FAIL` and a summary, exits nonzero if any fail.
- `configs/` — generated, gitignored. `MATRIX.txt` inside lists each config + its flag tuple.
- `.gitignore` — ignores `configs/`, `.last_err`, bytecode.

## How to run

```bash
bash tests/config_matrix/run.sh
```

Expected output:

```
PASS  matrix_00_cool_heat_step1p0.yaml
...
PASS  matrix_11_cool_heat_swingh_step0p5_ir.yaml
----------------------------------------
PASS=12 FAIL=0 of 12
```

To validate a single config by hand:

```bash
source /home/hoangminh/AgentsWork/Claude/HA/esphome/.venv/bin/activate
python3 tests/config_matrix/generate.py
esphome config tests/config_matrix/configs/matrix_08_nocool_heat_fanonly_quiet_fan5_swingh_step0p5_ir.yaml
```

## What a failure means

A `FAIL` here is a **schema or codegen bug** in `climate.py` — e.g. an option combination that
makes `to_code()` crash (a `select` not created, a missing ID), or a schema key rejected by
`cv`. This scenario does **not** test IR encode/decode logic or runtime behavior; use A2
(host unit tests) or the on-device scenarios for those.

## Current result

All 12 configs PASS against ESPHome `2026.7.0-dev` (the local editable install).