# clang-tidy findings — panaac component

Run with the ESPHome platform `.clang-tidy` config (clang-tidy 22.1.7), flags taken from
a real `ac-test` PlatformIO build (`idedata`), header analysis scoped to the panaac
component so toolchain/esphome-core noise is suppressed. `WarningsAsErrors: '*'` is set by
the platform config, so every finding is reported as `error:`.

Deduplicated across the `panaac.cpp` and `extra.cpp` translation units.

## Confirmed bugs / real issues

| File:line | Check | Issue |
|-----------|-------|-------|
| `panaac.cpp:621` | `bugprone-unchecked-optional-access` | `this->fan_mode.value()` called without `has_value()` check. **UB if `fan_mode` is ever unset.** Confirms the review finding. |
| `panaac.cpp:344` | `performance-unnecessary-copy-initialization` | `auto raw_data = data.get_raw_data();` copies the whole `std::vector<int>` on every IR receive. Should be `const auto &raw_data = ...`. Runtime heap copy in a hot path (also an embedded-reliability smell). |
| `panaac.cpp:358` | `bugprone-branch-clone` | `if/else` with effectively identical branches in the receive length-check. |

## Platform-convention violations (style, would block an upstream PR)

| File:line(s) | Check | Fix |
|-------------|-------|-----|
| `definitions.h:26`, `panaac.h:23`, `panaac.cpp:19`, `extra.cpp:20`, `extra.h:22` | `modernize-concat-nested-namespaces` | `namespace esphome { namespace panaac {` → `namespace esphome::panaac {` |
| `definitions.h:30` (`TAG`) and `:108-128` (all `STR_*`) | `cppcoreguidelines-avoid-non-const-global-variables` | `static const char *X = "..."` → `static const char *const X = "..."` (make the pointer const). The platform uses `const char *const` for these. |
| `panaac.h:67-68` | `readability-identifier-naming` (ProtectedMethodSuffix `_`) | Protected non-override methods `decode_data` / `decode_state` must end with `_` per platform convention (e.g. `decode_data_`). Override methods (`setup`, `transmit_state`, `on_receive`, `traits`) are exempt (VirtualMethodSuffix is empty). |
| `panaac.cpp:217` | `readability-simplify-boolean-expr` | The negated `&&` chain in the protocol-header check can be DeMorgan-simplified. |
| `panaac.cpp:582`, `:597` | `modernize-loop-convert` + `bugprone-too-small-loop-variable` | Index loops over `first_frame`/`second_frame` → use range-based `for (auto b : frame)`; the `uint8_t` index is narrower than `size_type`. |

## Notes

- The `extra.cpp` TU re-emits the `definitions.h`/`panaac.h` header findings (expected — the
  header is included by both). Dedup by file:line.
- clang-tidy analyzes headers under `esphome/components/panaac/.*` only; findings in esphome
  core or the toolchain are intentionally suppressed (those belong to the platform, not this
  component).
- None of these are fixed here — this branch only adds the analysis tooling and report.
  Fixes belong on a dedicated fix branch (out of scope for the test scenario).