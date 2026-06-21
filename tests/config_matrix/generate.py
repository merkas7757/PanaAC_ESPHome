#!/usr/bin/env python3
"""Generate a curated matrix of panaac config YAMLs for off-device validation.

Each generated config exercises a different combination of the panaac component's
boolean flags + temp_step, so `esphome config` can confirm the schema and codegen
(to_code: select creation, options) work across the board. Configs intentionally
omit wifi/api/ota so no secrets are required and no toolchain is needed.

Run:  python3 generate.py
Output: ./configs/matrix_<NN>_<slug>.yaml
"""
from __future__ import annotations
import os

HERE = os.path.dirname(os.path.abspath(__file__))

# (supports_cool, supports_heat, supports_fan_only, supports_quiet, fan_5level,
#  swing_horizontal, temp_step, ir_control)
CASES = [
    # 0 - all defaults
    (True,  True, False, False, False, False, 1.0, False),
    # 1 - fan 5 levels
    (True,  True, False, False, True,  False, 1.0, False),
    # 2 - quiet
    (True,  True, False, True,  False, False, 1.0, False),
    # 3 - 5 levels + quiet (7 select options)
    (True,  True, False, True,  True,  False, 1.0, False),
    # 4 - horizontal swing on
    (True,  True, False, False, False, True,  1.0, False),
    # 5 - temp step 0.5
    (True,  True, False, False, False, False, 0.5, False),
    # 6 - ir_control true (non-invasive, 38kHz carrier)
    (True,  True, False, False, False, False, 1.0, True),
    # 7 - fan_only + heat, cool off
    (False, True, True,  False, False, False, 1.0, False),
    # 8 - everything on
    (False, True, True,  True,  True,  True,  0.5, True),
    # 9 - minimal: only cool, nothing else
    (True,  False, False, False, False, False, 1.0, False),
    # 10 - horizontal + 5level + quiet
    (True,  True, False, True,  True,  True,  1.0, False),
    # 11 - ir_control true + horizontal (non-invasive full)
    (True,  True, False, False, False, True,  0.5, True),
]

HEADER = """\
esphome:
  name: "panaac-matrix-{idx:02d}"
  friendly_name: "PanaAC Matrix {idx:02d}"
  min_version: 2025.9.0
  name_add_mac_suffix: false

external_components:
  - source: ../../../esphome/components

esp8266:
  board: esp01_1m

logger:

remote_receiver:
  pin:
    number: GPIO4
    inverted: true
    mode:
      output: true
      open_drain: true
    allow_other_uses: true
  tolerance: 55%
  id: ir_receiver
  idle: 5ms

remote_transmitter:
  carrier_duty_percent: 50%
  pin:
    number: GPIO4
    inverted: true
    mode:
      output: true
      open_drain: true
    allow_other_uses: true

climate:
  - platform: panaac
    name: "PanaAC Matrix {idx:02d}"
    receiver_id: ir_receiver
"""

def slug(c):
    cool, heat, fan_only, quiet, fan5, swing_h, tstep, ir = c
    parts = []
    parts.append("cool" if cool else "nocool")
    parts.append("heat" if heat else "noheat")
    if fan_only: parts.append("fanonly")
    if quiet: parts.append("quiet")
    if fan5: parts.append("fan5")
    if swing_h: parts.append("swingh")
    parts.append(f"step{str(tstep).replace('.', 'p')}")
    if ir: parts.append("ir")
    return "_".join(parts)

def flags_block(c):
    cool, heat, fan_only, quiet, fan5, swing_h, tstep, ir = c
    lines = []
    lines.append(f"    supports_cool: {'true' if cool else 'false'}")
    lines.append(f"    supports_heat: {'true' if heat else 'false'}")
    lines.append(f"    supports_fan_only: {'true' if fan_only else 'false'}")
    lines.append(f"    supports_quiet: {'true' if quiet else 'false'}")
    lines.append(f"    fan_5level: {'true' if fan5 else 'false'}")
    lines.append(f"    swing_horizontal: {'true' if swing_h else 'false'}")
    lines.append(f"    temp_step: {tstep}")
    lines.append(f"    ir_control: {'true' if ir else 'false'}")
    return "\n".join(lines)

def main():
    out_dir = os.path.join(HERE, "configs")
    os.makedirs(out_dir, exist_ok=True)
    manifest = []
    for i, c in enumerate(CASES):
        body = HEADER.format(idx=i) + flags_block(c) + "\n"
        fname = f"matrix_{i:02d}_{slug(c)}.yaml"
        with open(os.path.join(out_dir, fname), "w") as f:
            f.write(body)
        manifest.append((fname, c))
    with open(os.path.join(HERE, "configs", "MATRIX.txt"), "w") as f:
        for fname, c in manifest:
            f.write(f"{fname}\t{c}\n")
    print(f"Generated {len(manifest)} configs in {out_dir}")

if __name__ == "__main__":
    main()