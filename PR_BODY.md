## Summary

Fixes #16.

Adds an optional `fan_only_switch:` helper entity for users who bridge this climate through HomeKit. HomeKit does not expose a native `FAN_ONLY` climate mode, so the switch provides a bridge-friendly control that toggles the AC into and out of `FAN_ONLY` while keeping the climate entity state synchronized.

## Scope

- `esphome/components/panaac/climate.py`
  - adds `fan_only_switch:` and `fan_only_switch_id:` schema support
  - registers a companion switch only when both `supports_fan_only: true` and `fan_only_switch: true` are set
- `esphome/components/panaac/definitions.h`, `extra.h`, `extra.cpp`
  - introduces a `PanaACFanOnlySwitch` helper entity
- `esphome/components/panaac/panaac.h`, `panaac.cpp`
  - tracks the last non-`FAN_ONLY` operating mode
  - keeps the helper switch synchronized on local updates and received IR state changes
- `README.md`
  - documents the new optional config and its HomeKit use case

## Design

- The fix stays inside this component instead of depending on Home Assistant or HomeKit changes.
- The helper is opt-in to avoid adding extra entities for users who do not need HomeKit workarounds.
- Turning the switch on sets `FAN_ONLY` mode.
- Turning the switch off restores the last non-`FAN_ONLY` operating mode when possible, which keeps the UX closer to a mode toggle than a hard power-off.
- The switch publishes its state from both outbound updates and IR receive events so Home Assistant and HomeKit stay aligned.

## Changes

- Added `switch` auto-loading and config registration for the helper entity.
- Added runtime synchronization between the climate state and the switch state.
- Added restore logic for leaving `FAN_ONLY` mode cleanly.
- Updated the README examples to show `fan_only_switch: true`.

## Verification

- Ran `python -m py_compile esphome/components/panaac/climate.py`
- Ran `git diff --check`

PR by Codex
