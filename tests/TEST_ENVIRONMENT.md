# PanaAC_ESPHome — Testing

This is the **consolidated testing branch** (`feature/test-all`). It collects all 8 test
scenarios (each originally developed on its own `feature/test-*` branch) into one place under
`tests/`, plus this overview. Each scenario also has its own `TEST_ENVIRONMENT.md` with the
exact hardware/steps for that scenario.

The scenarios run from easiest to hardest. The first three are **off-device** (no ESP, no IR
hardware) and can run right now on this machine. The remaining five are **on-device** and need
hardware you must prepare.

## One-time environment setup (off-device scenarios)

The off-device tests use the ESPHome toolchain (the `esphome` CLI) and a C++ compiler. Set up
the workspace dev venv once — full details are in the workspace `DEV_ENVIRONMENT.md`:

```bash
# Python 3.13 venv at the workspace root (NOT this repo) with esphome installed editable
# from the sibling esphome/ platform source.
python3.13 -m venv /home/hoangminh/AgentsWork/Claude/HA/esphome/.venv
source /home/hoangminh/AgentsWork/Claude/HA/esphome/.venv/bin/activate
pip install --upgrade pip setuptools wheel
pip install -e /home/hoangminh/AgentsWork/Claude/HA/esphome/esphome
```

The run scripts default to that venv; override with `PANAAC_VENV=/path/to/venv` if yours lives
elsewhere. Additional per-scenario needs:

- **A3 (clang-tidy)**: `pip install "clang-tidy==22.1.7"` and a compiled panaac build to supply
  PlatformIO `idedata` (run `esphome compile esphome/ac-test-1.yaml` once, then
  `pio run -t idedata -e ac-test` in its `.esphome/build/ac-test` dir).
- **A2 (host unit tests)**: a host `g++` (standard on this machine). No venv needed for A2
  itself, but having the venv doesn’t hurt.

The component under test lives in `esphome/components/panaac/` (this repo). Test configs pull it
in via `external_components: source: ../../esphome/components` (relative to each test YAML).

## Scenario index

### Off-device (run now)

| # | Scenario | Dir | What it tests | Run |
|---|----------|-----|---------------|-----|
| A1 | config validation matrix | `tests/config_matrix/` | `CONFIG_SCHEMA` + `to_code` (select creation/options) across all flag combos | `bash tests/config_matrix/run.sh` |
| A3 | static analysis | `tests/clang_tidy/` | platform `.clang-tidy` rules; confirms `fan_mode.value()` unchecked + a vector copy in `on_receive` | `python3 tests/clang_tidy/run_clang_tidy.py` (see `FINDINGS.md`) |
| A2 | host protocol unit tests | `tests/host_unit_tests/` | encode/decode/checksum/round-trip on the **unmodified** component (stubbed ESPome); characterizes the L2-loss bug | `bash tests/host_unit_tests/build.sh` |

Run all three at once:

```bash
bash tests/run_all.sh
```

### On-device (need hardware)

| # | Scenario | Dir | Hardware | Run |
|---|----------|-----|----------|-----|
| B3 | real-remote receive | `tests/on_device_receive/` | 1 ESP + IR demodulator + Panasonic remote | `bash tests/on_device_receive/run.sh` |
| B1 | single-board loopback | `tests/on_device_loopback/` | 1 ESP (shared GPIO4, nothing else) | `bash tests/on_device_loopback/run.sh` |
| B2 | two-board optical loopback | `tests/on_device_optical/` | 2 ESP + IR LED (+transistor) + demodulator | `bash tests/on_device_optical/run.sh sender` / `receiver` |
| B4 | real AC end-to-end | `tests/on_device_real_ac/` | 1 ESP wired to the AC (or IR LED pointed at it) + the AC | `bash tests/on_device_real_ac/run.sh` |
| B5 | HA entity integration | `tests/on_device_ha/` | 1 ESP (shared GPIO4) + Home Assistant | `bash tests/on_device_ha/run.sh` |

Each on-device dir has a `TEST_ENVIRONMENT.md` with the exact wiring, prerequisites, steps, and
pass/fail criteria. `secrets.yaml` (Wi-Fi creds) is gitignored; each `run.sh` creates a dummy
one if missing — replace with real values only if you intend to bring the device up on Wi-Fi.

## Recommended order

1. `bash tests/run_all.sh` — off-device baseline (A1/A3/A2). Everything should pass; A2’s
   `transmit_state_loses_level2` test asserts the *current buggy* L2-loss behavior (it passes
   today; flip the expectation to `LEVEL_2` once that bug is fixed).
2. **B3** — cheapest on-device check; validates the receive path with a real remote.
3. **B1** — strongest single-board test; validates encode→receive→decode round-trip on
   hardware (no extra parts).
4. **B5** — confirms the HA entity surface (climate + 3 selects, option lists per flag).
5. **B2** — validates the real 38 kHz optical path (non-invasive deployment method).
6. **B4** — final acceptance: the real Panasonic AC responds to commands.

## Notes

- The component source is **not modified** on this branch (the stubs in `tests/host_unit_tests`
  shadow the ESPome headers; `panaac.cpp`/`extra.cpp`/`definitions.h`/`panaac.h`/`extra.h` are
  unchanged). Fixes belong on separate `bugfix/*` branches.
- `tests/run_all.sh` skips A3 if `clang-tidy` or the `idedata.json` isn’t present, and skips A2
  if `g++` isn’t present, so it degrades gracefully on a minimal machine.