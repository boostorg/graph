#!/usr/bin/env python3
"""Print a markdown table comparing compiler-warning counts between two CI log archives.

Usage: warn_table.py BASELINE.zip AFTER.zip

A warning is a line matching 'file:line:col: warning:' (clang/gcc) or 'warning Cxxxx'
(msvc). Linker, "N warnings generated" summary, and CI-runner lines are not counted.
"""
import re
import sys
import zipfile

WARNING = re.compile(r":\d+:\d+: warning:|warning C\d+")


def counts(zip_path):
    """Map each CI job to its warning count, read from the run's log archive."""
    result = {}
    with zipfile.ZipFile(zip_path) as archive:
        for entry in archive.namelist():
            # Top-level per-job logs are named like "12_ubuntu (gcc-14, 14).txt".
            if "/" in entry or not entry.endswith(".txt"):
                continue
            job = re.sub(r"^\d+_", "", entry[:-4])
            log = archive.read(entry).decode("utf-8", "replace")
            result[job] = sum(1 for line in log.splitlines() if WARNING.search(line))
    return result


def main(baseline_zip, after_zip):
    baseline = counts(baseline_zip)
    after = counts(after_zip)
    print("| Job | Baseline | After | Delta |")
    print("|-----|---------:|------:|------:|")
    for job in sorted(baseline.keys() | after.keys()):
        if "cmake" in job:  # cmake jobs build only the library, they never warn
            continue
        before = baseline.get(job, 0)
        now = after.get(job, 0)
        diff = now - before
        delta = f"{diff:+d}" if diff else "0"
        print(f"| {job} | {before} | {now} | {delta} |")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit("usage: warn_table.py BASELINE.zip AFTER.zip")
    main(sys.argv[1], sys.argv[2])
