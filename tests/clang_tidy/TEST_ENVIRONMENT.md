# Scenario A3 — Off-device static analysis (clang-tidy)

**Difficulty:** easy · **Hardware required:** none · **What it tests:** static-conformance of
the panaac C++ to the ESPHome platform's `.clang-tidy` rules — catches real bugs (unchecked
optional access, unnecessary copies, branch-clone) and platform style violations (namespace
form, const-correctness of globals, protected-method naming, loop modernization).

## What you need to prepare

Fully off-device. Requirements:

1. **ESPHome dev venv** at `/home/hoangminh/AgentsWork/Claude/HA/esphome/.venv`
   (Python 3.13, esphome installed editable). See workspace `DEV_ENVIRONMENT.md`.
2. **clang-tidy 22.1.7** installed in the venv:
   ```bash
   source /home/hoangminh/AgentsWork/Claude/HA/esphome/.venv/bin/activate
   pip install "clang-tidy==22.1.7"   # also in esphome/requirements_dev.txt
   ```
3. **A compiled panaac build** to supply real compile flags. The wrapper reads PlatformIO's
   `idedata` from the build dir. Build once (any panaac config), then dump idedata:
   ```bash
   source /home/hoangminh/AgentsWork/Claude/HA/esphome/.venv/bin/activate
   cd <repo>/esphome
   esphome compile ac-test-1.yaml                      # produces .esphome/build/ac-test
   cd .esphome/build/ac-test
   pio run -t idedata -e ac-test >/dev/null 2>&1        # writes .pio/build/ac-test/idedata.json
   ```
   No ESP device, no network (after the initial build) is needed.

No secrets are required (the build config used here is `ac-test-1.yaml`; its `secrets.yaml`
is gitignored and dummy values are fine — only needed at compile time).

## Files

- `run_clang_tidy.py` — trimmed port of `esphome/script/clang-tidy`: reads `idedata.json`,
  applies the platform's Xtensa flag-stripping + pgmspace stubs, forces `gnu++20`, and runs
  clang-tidy with the platform `.clang-tidy` config over `panaac.cpp` + `extra.cpp`.
  Header analysis is scoped to `esphome/components/panaac/.*` so only the component's own
  headers are reported (toolchain + esphome-core noise suppressed), mirroring the platform's
  `--header-filter` approach.
- `FINDINGS.md` — deduplicated findings: confirmed bugs + platform-convention violations.
- `clang_tidy_report.txt` — raw per-file clang-tidy output (generated, gitignored).

## How to run

```bash
source /home/hoangminh/AgentsWork/Claude/HA/esphome/.venv/bin/activate
cd <repo>
python3 tests/clang_tidy/run_clang_tidy.py
```

Defaults assume the `ac-test` build and the local platform path. Override:

```bash
python3 tests/clang_tidy/run_clang_tidy.py <idedata.json> <component-dir>
```

Exit code is nonzero if any finding is reported. Per-file findings are printed to stdout; the
full raw output is in `clang_tidy_report.txt`.

## What a finding means

- **`bugprone-*` / `performance-*`** findings are real bugs or hot-path issues in the
  component (e.g. `bugprone-unchecked-optional-access` on `fan_mode.value()`).
- **`readability-identifier-naming` / `modernize-*` / `cppcoreguidelines-*`** findings are
  platform-convention violations that would block merging the component upstream. They are
  style, not crashes — but the platform's CI enforces them via `WarningsAsErrors: '*'`.

This scenario does **not** test IR encode/decode behavior or runtime logic; use A2 (host unit
tests) or the on-device scenarios for those.

## Current result

clang-tidy 22.1.7 reports findings on the panaac component — see `FINDINGS.md` for the
deduplicated list. Notably it independently confirms the unchecked-optional-access on
`fan_mode.value()` and an unnecessary `std::vector` copy in `on_receive`.