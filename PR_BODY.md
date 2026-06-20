## Summary

Fixes #14.

Adds optional POWERFUL and ECO preset support to the Panasonic AC climate component. The feature is exposed through ESPHome climate presets and a companion preset select entity, with protocol encode/decode support for the relevant Panasonic IR bits.

## Scope

- `esphome/components/panaac/climate.py`
  - adds `supports_powerful:` and `supports_eco:` schema options
  - registers a companion preset select when either option is enabled
- `esphome/components/panaac/definitions.h`
  - defines preset strings, state storage, and Panasonic bit positions/masks
- `esphome/components/panaac/extra.h`, `extra.cpp`
  - adds the preset select entity implementation
- `esphome/components/panaac/panaac.h`, `panaac.cpp`
  - advertises supported presets in climate traits
  - decodes POWERFUL/ECO from received IR frames
  - encodes POWERFUL/ECO into transmitted IR frames
  - normalizes preset behavior for unsupported modes and fan changes
- `README.md`
  - documents the new configuration options

## Design

- Presets map onto ESPHome's built-in climate preset model instead of inventing a separate control surface.
- The feature is opt-in because not every Panasonic remote exposes POWERFUL/ECO buttons.
- POWERFUL and ECO are cleared automatically when the climate is off or in unsupported operating modes.
- POWERFUL is also cleared when the fan mode changes, matching the protocol behavior discussed in the issue.
- A separate preset select keeps the feature available even in frontends that do not surface climate presets prominently.

## Changes

- Added preset trait advertisement for `None`, `Boost`, and `Eco` as supported.
- Added runtime preset synchronization between climate state, helper select state, and IR receive updates.
- Added Panasonic frame bit handling for both transmit and receive paths.
- Updated README examples to show `supports_powerful` and `supports_eco`.

## Verification

- Ran `python -m py_compile esphome/components/panaac/climate.py`
- Ran `git diff --check`

PR by Codex
