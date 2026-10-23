#!/usr/bin/env python3
"""Print a markdown table of Boost dependency changes between two CI log archives.

Usage: dep_table.py BASELINE.zip AFTER.zip

Each archive is a GitHub Actions run-log zip. The `deps` job of the
dependency-report workflow prints two marker-delimited boostdep reports into
its log:

  ===DEP-BRIEF-START===    boostdep --brief graph      ===DEP-BRIEF-END===
  ===DEP-PRIMARY-START===  boostdep graph (primary)    ===DEP-PRIMARY-END===

From those it derives two metrics and prints their delta vs the baseline:
  - the set of transitive Boost modules graph depends on (from --brief:
    Primary + Secondary sections = the full transitive closure), and
  - the header-inclusion weight of each direct dependency (from the primary
    report: how many distinct boost/graph files pull that dependency in).
Weights are the live signal, they drop toward zero as coupling is removed;
the module count is the coarse target that only moves when a dep hits zero.
"""
import re
import sys
import zipfile

BRIEF = ("===DEP-BRIEF-START===", "===DEP-BRIEF-END===")
PRIMARY = ("===DEP-PRIMARY-START===", "===DEP-PRIMARY-END===")
# Module names as boostdep prints them, e.g. "numeric~conversion".
MODULE = re.compile(r"[A-Za-z0-9_.~-]+")
TIMESTAMP = re.compile(r"^\d{4}-\d\d-\d\dT[\d:.]+Z\s?")  # GitHub log line prefix
ANSI = re.compile(r"\x1b\[[0-9;]*m")


def read_clean_lines(zip_path):
    """Top-level per-job logs, with GitHub timestamp and ANSI prefixes stripped."""
    lines = []
    with zipfile.ZipFile(zip_path) as z:
        for entry in z.namelist():
            if "/" in entry or not entry.endswith(".txt"):
                continue
            for ln in z.read(entry).decode("utf-8", "replace").splitlines():
                lines.append(TIMESTAMP.sub("", ANSI.sub("", ln)))
    return lines


def block(lines, markers):
    """Lines strictly between the marker lines, matched exactly.

    Exact matching is what skips the echoed `echo "<marker>"` command lines
    GitHub prepends to a run step, so we capture boostdep's real stdout.
    """
    start, end = markers
    out, capturing = [], False
    for ln in lines:
        s = ln.strip()
        if s == start:
            capturing = True
            continue
        if s == end and capturing:
            break
        if capturing:
            out.append(ln)
    return out


def parse_brief(lines):
    """--brief output -> set of module names (one bare name per line)."""
    mods = set()
    for ln in lines:
        s = ln.strip()
        if not s or s.startswith("#") or s.lower().startswith("brief dependency"):
            continue
        if MODULE.fullmatch(s):
            mods.add(s)
    return mods


def parse_weights(lines):
    """Primary report -> {dependency: number of distinct graph files that pull it in}."""
    weights, cur = {}, None
    for ln in lines:
        head = re.match(r"^([A-Za-z0-9_.~-]+):\s*$", ln)  # "module:" at column 0
        if head:
            cur = head.group(1)
            weights.setdefault(cur, set())
            continue
        frm = re.match(r"^\s+from\s+(.+?)\s*$", ln)
        if frm and cur is not None:
            weights[cur].add(frm.group(1))
    return {k: len(v) for k, v in weights.items()}


def main(base_zip, pr_zip):
    bl, pl = read_clean_lines(base_zip), read_clean_lines(pr_zip)
    base_mods = parse_brief(block(bl, BRIEF))
    pr_mods = parse_brief(block(pl, BRIEF))
    base_w = parse_weights(block(bl, PRIMARY))
    pr_w = parse_weights(block(pl, PRIMARY))

    # Header-inclusion weights: the live signal. Only rows that changed, most-reduced first.
    print("**Header-inclusion weights** (graph files pulling each direct dependency in):")
    print()
    changed = []
    for dep in base_w.keys() | pr_w.keys():
        b, p = base_w.get(dep, 0), pr_w.get(dep, 0)
        if b != p:
            changed.append((p - b, dep, b, p))
    if changed:
        print("| Dependency | develop | PR | Δ |")
        print("|-----|--------:|---:|----:|")
        for d, dep, b, p in sorted(changed):  # reductions (negative delta) first
            print(f"| {dep} | {b} | {p} | {d:+d} |")
    else:
        print("_No header-inclusion-weight changes._")
    print()

    # Transitive module set: the coarse target.
    added = sorted(pr_mods - base_mods)
    removed = sorted(base_mods - pr_mods)
    nb, np_ = len(base_mods), len(pr_mods)
    diff = np_ - nb
    print(f"**Transitive Boost modules:** {nb} → {np_} ({f'{diff:+d}' if diff else '0'})")
    if added:
        print(f"- added: {', '.join(added)}")
    if removed:
        print(f"- removed: {', '.join(removed)}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit("usage: dep_table.py BASELINE.zip AFTER.zip")
    main(sys.argv[1], sys.argv[2])
