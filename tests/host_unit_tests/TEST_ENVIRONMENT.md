# Scenario A2 — Off-device host protocol unit tests

**Difficulty:** medium · **Hardware required:** none · **What it tests:** the panaac IR
**encode/decode/checksum/mapping logic** — the bug-prone core — by compiling the **unmodified**
component on the host and exercising it directly. Catches the kind of bugs compilation and
config validation cannot (state↔byte mapping, round-trip consistency, the fan L2/L4 loss).

## Approach (and why no component refactor)

The component's encode/decode logic lives inside `PanaACClimate` methods, which inherit the
ESPHome runtime (`ClimateIR` → `Component`/`Climate`/`RemoteReceiverListener`/`RemoteTransmittable`,
plus `select::Select`, `remote_base::RemoteReceiveData`, `FixedVector`, `ClimateTraits`, …).
That runtime targets Arduino/ESP-IDF and cannot be linked on a host.

Two ways to host-test the real logic:

1. **Refactor** the protocol logic out of the class into free functions (modifies the component).
2. **Stub** the ESPome headers so the unmodified component compiles + links on the host.

Per the project rule "refactor only if there is no other choice", this scenario uses **option 2**
— a minimal stub include-directory that shadows `esphome/...` headers — so the component source
(`panaac.cpp`, `extra.cpp`, `definitions.h`, `panaac.h`, `extra.h`) is **not modified at all**.
The stubs are faithful where it matters: `RemoteReceiveData` is a verbatim reimplementation of
the real mark/space/tolerance/`expect_item` logic, and the climate enums are copied from the
platform with identical values.

## What you need to prepare

Fully off-device. Requirements:

1. A **C++17/20 compiler** (`g++`) — standard on the WSL2 machine.
2. The component source at `esphome/components/panaac/` (unchanged).
3. No venv, no PlatformIO, no network, no ESPHome install — the stubs replace the platform.

## Files

- `stubs/esphome/...` — minimal ESPHome header stubs (`core/log.h`, `core/helpers.h` (FixedVector),
  `core/component.h`, `core/entity_base.h`, `core/optional.h`, `components/climate/*`,
  `components/climate_ir/climate_ir.h`, `components/select/*`, `components/remote_base/remote_base.h`,
  `components/sensor/sensor.h`). Placed first on the include path so they shadow the real platform.
- `test_framework.h` — tiny CHECK_EQ/CHECK_TRUE/CHECK_FALSE macros (no doctest dependency).
- `test_main.cpp` — the tests + a `TestClimate : PanaACClimate` subclass that re-exposes the
  protected `transmit_state` / `on_receive` / `decode_data` / `decode_state` via `using`
  (a test-only subclass; the component itself is untouched).
- `build.sh` — compiles `test_main.cpp` + `panaac.cpp` + `extra.cpp` against the stubs and runs.

## How to run

```bash
bash tests/host_unit_tests/build.sh
```

Expected:

```
>> compiling
>> running
----
run=26 failed=0
```

## What the tests cover

| Test | Checks |
|------|--------|
| `encode_raw_size` | `transmit_data` emits a full 27-byte (440-entry) frame |
| `encode_spec_bytes` | Encoded bytes match the protocol spec: header `02 20 E0 04 00`, power+mode byte, temp byte, fan+swingV byte, additive checksum |
| `round_trip` | encode → `on_receive` decode reproduces the encoded state (mode, fan_level, swing_v/h, half-degree temp) |
| `select_keeps_level2` | Selecting "Level 2" via the fan-level select survives `update_state` + round-trips back as L2 |
| `transmit_state_loses_level2` | **Characterizes a known bug**: the climate `transmit_state` path collapses `FAN_LOW → L1`, discarding a prior L2. Asserts the current (buggy) behavior so the suite stays green; flip to `LEVEL_2` when fixed |
| `decode_rejects_bad_frames` | `decode_state` rejects wrong length, wrong protocol header, and bad checksum; accepts a valid one |

## Limitations / future work

- Reference IR vectors from IRremoteESP8266 (known-good Panasonic AC captures) are not vendored;
  encode correctness is instead checked against the hand-computed spec and round-trip
  consistency. Adding IRremoteESP8266 vectors would make the decode test independently grounded.
- The climate `control()`/`ClimateCall` path is not stubbed, so the `transmit_state` L2-loss is
  driven via the exposed protected method (a test-only `using`), not through a real climate call.
- The stubs model only the API surface panaac touches; they are not a general ESPHome host shim.