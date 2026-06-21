#!/usr/bin/env python3
"""Run the ESPHome platform clang-tidy over the panaac component sources.

This is a trimmed port of esphome/script/clang-tidy: it takes the real compile
flags from a PlatformIO `idedata` dump of an already-built panaac config
(`.esphome/build/<name>/.pio/build/<name>/idedata.json`), applies the same
GCC-only-flag stripping + Xtensa host-compile shim the platform uses, and runs
the pip-installed clang-tidy with the platform's `.clang-tidy` config.

It does NOT modify the component; it only reads it. Findings are printed and
also written to clang_tidy_report.txt.

Usage:
  python3 run_clang_tidy.py [<idedata.json>] [<component-dir>]

Defaults:
  idedata  = <repo>/esphome/.esphome/build/ac-test/.pio/build/ac-test/idedata.json
  component= <repo>/esphome/components/panaac
"""
from __future__ import annotations
import json
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
PLAT = os.environ.get("ESPHOME_PLATFORM",
                       "/home/hoangminh/AgentsWork/Claude/HA/esphome/esphome")

DEFAULT_IDEDATA = os.path.join(REPO, "esphome/.esphome/build/ac-test",
                               ".pio/build/ac-test/idedata.json")
DEFAULT_COMPONENT = os.path.join(REPO, "esphome/components/panaac")

# GCC-only flags the platform's clang-tidy script drops (must match its list).
OMIT_FLAGS = {
    "-free", "-fipa-pta", "-fstrict-volatile-bitfields", "-mlongcalls",
    "-mtext-section-literals", "-mdisable-hardware-atomics",
    "-mfix-esp32-psram-cache-issue", "-mfix-esp32-psram-cache-strategy=memw",
    "-fno-tree-switch-conversion", "-freorder-blocks", "-fno-jump-tables",
    "-fno-shrink-wrap", "-mno-target-align",
}


def build_flags(idedata: dict) -> list[str]:
    cxx_path = idedata["cxx_path"]
    triplet = os.path.basename(cxx_path)[:-4]  # strip "-g++"
    cmd: list[str] = []
    if triplet.startswith("xtensa-"):
        # Xtensa: compile in 32-bit x86, pretend to be Xtensa (see platform script).
        cmd += ["-m32", "-U__i386__", "-U__x86_64__",
                "-D__XTENSA__", "-D__XTENSA_EL__", "-D_LIBC"]
    else:
        cmd += [f"--target={triplet}", "-Qunused-arguments"]

    cmd += [
        "-nostdinc++", "-nostdinc",
        # Replace pgmspace.h (GNU extensions clang can't handle).
        "-D_PGMSPACE_H_",
        "-Dpgm_read_byte(s)=(*(const uint8_t *)(s))",
        "-Dpgm_read_byte_near(s)=(*(const uint8_t *)(s))",
        "-Dpgm_read_word(s)=(*(const uint16_t *)(s))",
        "-Dpgm_read_dword(s)=(*(const uint32_t *)(s))",
        "-Dpgm_read_ptr(s)=(*(const void *const *)(s))",
        "-DPROGMEM=", "-DPGM_P=const char *", "-DPSTR(s)=(s)", "-DPSTRN(s, n)=(s)",
        "-Ddeprecated(x)=",
        "-DCLANG_TIDY",            # code can condition on clang-tidy presence
        "-D_GLIBCXX_HAVE_TLS",     # fix __once_callable in libstdc++ headers
        "-std=gnu++20",            # match the real esphome build
    ]

    for f in idedata.get("cxx_flags", []):
        if f in OMIT_FLAGS:
            continue
        if f.startswith("-Werror"):   # .clang-tidy WarningsAsErrors governs this
            continue
        if f.startswith("-std="):
            continue
        cmd.append(f)

    for d in idedata.get("defines", []):
        cmd.append(f"-D{d}")

    inc = idedata.get("includes", {})
    for key in ("build", "toolchain"):
        for p in inc.get(key, []):
            cmd.append(f"-I{p}")
    return cmd


def main() -> int:
    idedata_path = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_IDEDATA
    component_dir = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_COMPONENT
    if not os.path.isfile(idedata_path):
        print(f"ERROR: idedata not found at {idedata_path}.\n"
              f"Build a panaac config first (esphome compile esphome/ac-test-1.yaml), "
              f"then `pio run -t idedata -e ac-test` in its .esphome/build dir.", file=sys.stderr)
        return 2

    idedata = json.load(open(idedata_path))
    flags = build_flags(idedata)

    clang_tidy = shutil.which("clang-tidy")
    if not clang_tidy:
        print("ERROR: clang-tidy not on PATH. pip install clang-tidy==22.1.7", file=sys.stderr)
        return 2

    cfg = os.path.join(PLAT, ".clang-tidy")
    sources = [os.path.join(component_dir, f) for f in ("panaac.cpp", "extra.cpp")]
    sources = [s for s in sources if os.path.isfile(s)]
    if not sources:
        print(f"ERROR: no .cpp sources in {component_dir}", file=sys.stderr)
        return 2

    report = os.path.join(HERE, "clang_tidy_report.txt")
    all_ok = True
    # Only report diagnostics from panaac-owned headers (the component under test).
    # Matches the platform's --header-filter approach but scoped to this component,
    # so toolchain + esphome-core header noise is suppressed.
    header_filter = os.path.abspath(component_dir) + "/.*"
    with open(report, "w") as rep:
        for src in sources:
            cmd = [clang_tidy, f"--config-file={cfg}",
                   f"--header-filter={header_filter}", src, "--", *flags]
            rep.write(f"$ {clang_tidy} --config-file={cfg} {src} -- <flags...>\n")
            r = subprocess.run(cmd, capture_output=True, text=True)
            rep.write(r.stdout)
            rep.write(r.stderr)
            rep.write("\n")
            # Print a compact per-file summary
            findings = [ln for ln in r.stdout.splitlines()
                        if ":" in ln and ("warning:" in ln or "error:" in ln)]
            print(f"=== {os.path.basename(src)} ===")
            if findings:
                for ln in findings:
                    print(ln)
                all_ok = False
            else:
                print("no findings")
    print(f"\nFull report: {report}")
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())