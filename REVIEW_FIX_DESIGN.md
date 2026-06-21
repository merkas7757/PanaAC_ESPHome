# Review Findings And Fix Design

This change addresses three behavioral issues found during review of the `panaac` custom component.

## 1. Boot-time unsolicited IR transmit

### Finding

`PanaACClimate::setup()` initialized a default climate state and immediately called `transmit_state()`. On every reboot,
the controller could send a fresh IR command even when the user did not request one.

### Fix

- Keep `ClimateIR::setup()` so ESPHome restore logic still runs.
- Replace the boot-time `transmit_state()` call with local state synchronization only.
- Publish the restored/default state to Home Assistant and the auxiliary select entities without transmitting IR.

### Design intent

Boot should be state restoration, not device actuation. IR output now happens only on explicit user control or on
received remote-state synchronization.

## 2. 5-level fan selection was lost on normal climate updates

### Finding

The component exposes detailed fan levels through a select entity, but `transmit_state()` rebuilt `fan_level` only from
coarse ESPHome `fan_mode`. Any later temperature or mode change from the main climate card reset `Level 2` to `Level 1`
and `Level 4` to `Level 3`.

### Fix

- Add a shared `sync_state_from_climate_()` helper.
- Preserve the existing detailed fan level when it is compatible with the current fan mode.
- Fall back to default representative levels only when the stored detailed level is incompatible.

### Design intent

The main climate entity and the fan-level select should cooperate instead of overwriting each other.

## 3. `temp_step` accepted invalid values

### Finding

The schema accepted any float even though the Panasonic encoder only supports whole-degree and half-degree temperatures.

### Fix

- Restrict `temp_step` in `climate.py` to `0.5` or `1.0`.

### Design intent

Validation should reject UI states that the IR protocol cannot encode exactly.
