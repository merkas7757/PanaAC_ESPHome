## Summary

Fixes #15.

Passes the climate entity's `device_id:` through to the companion `select` entities so they stay grouped with the main climate under the same ESPHome sub-device in ESPHome and Home Assistant.

## Scope

- `esphome/components/panaac/climate.py`
  - imports `CONF_DEVICE_ID`
  - adds a small helper to build companion entity configs consistently
  - forwards `device_id:` to Fan Level, Swing Vertical, and Swing Horizontal selects when present
- `README.md`
  - documents the `device_id:` behavior for sub-device users

## Design

- The fix targets the root cause in code generation: helper entities were registered without the climate's `device_id:`.
- Existing configurations without `device_id:` keep their current behavior.
- The solution is intentionally narrow and does not rename entities or change unrelated Home Assistant presentation details.

## Changes

- Centralized companion entity config creation in `build_entity_config(...)`.
- Copied `device_id:` only when the parent climate config includes it.
- Added a short README note so the feature is discoverable.

## Verification

- Ran `python -m py_compile esphome/components/panaac/climate.py`
- Ran `git diff --check`

PR by Codex
