#!/usr/bin/env python3
"""Print a markdown table of Boost dependency changes between two CI log archives.

Usage: dep_table.py BASELINE.zip AFTER.zip

Each archive is a GitHub Actions run-log zip. The `deps` CI job prints two
marker-delimited boostdep reports into its log:

  ===DEP-BRIEF-START===    boostdep --brief graph      ===DEP-BRIEF-END===
  ===DEP-PRIMARY-START===  boostdep graph (primary)    ===DEP-PRIMARY-END===

From those it derives two metrics and prints their delta vs the baseline:
  - the set of transitive Boost modules graph depends on (from --brief), and
  - the header-inclusion weight of each direct dependency (from the primary
    report: how many distinct boost/graph/* headers pull that dependency in).
Weights are the live signal, they drop toward zero as coupling is removed;
the module count is the coarse target that only moves when a dep hits zero.
"""
import re
import sys
import zipfile

BRIEF = ("===DEP-BRIEF-START===", "===DEP-BRIEF-END===")
PRIMARY = ("===DEP-PRIMARY-START===", "===DEP-PRIMARY-END===")


def read_logs(zip_path):
    """Concatenate the top-level per-job .txt logs from a run-log archive."""
    out = []
    with zipfile.ZipFile(zip_path) as z:
        for entry in z.namelist():
            if "/" in entry or not entry.endswith(".txt"):
                continue
            out.append(z.read(entry).decode("utf-8", "replace"))
    return "\n".join(out)


def block(text, markers):
    m = re.search(re.escape(markers[0]) + r"(.*?)" + re.escape(markers[1]), text, re.S)
    return m.group(1) if m else ""


def parse_brief(text):
    """--brief output -> set of module names (one bare name per line)."""
    mods = set()
    for line in text.splitlines():
        s = line.strip()
        if not s or s.startswith("#") or s.lower().startswith("brief dependency"):
            continue
        if re.fullmatch(r"[A-Za-z0-9_.-]+", s):
            mods.add(s)
    return mods


def parse_weights(text):
    """Primary report -> {dependency: number of distinct boost/graph headers that pull it in}."""
    weights, cur = {}, None
    for line in text.splitlines():
        head = re.match(r"^([A-Za-z0-9_.-]+):\s*$", line)  # "module:" at column 0
        if head:
            cur = head.group(1)
            weights.setdefault(cur, set())
            continue
        frm = re.match(r"^\s+from\s+(.+?)\s*$", line)
        if frm and cur is not None:
            weights[cur].add(frm.group(1))
    return {k: len(v) for k, v in weights.items()}


def main(base_zip, pr_zip):
    base, pr = read_logs(base_zip), read_logs(pr_zip)
    base_mods = parse_brief(block(base, BRIEF))
    pr_mods = parse_brief(block(pr, BRIEF))
    base_w = parse_weights(block(base, PRIMARY))
    pr_w = parse_weights(block(pr, PRIMARY))

    # Header-inclusion weights: the live signal. Show only rows that changed,
    # most-reduced first.
    print("**Header-inclusion weights** (graph headers pulling each direct dependency in):")
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
