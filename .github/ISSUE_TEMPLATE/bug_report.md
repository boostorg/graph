---
name: Bug report
about: Report a defect or unexpected behavior in Boost.Graph
title: ''
labels: ''
type: Bug
assignees: ''
---

<!--
Thank you for taking the time to file a bug report. Please fill in as much
of the template below as you can. Reports without a reproducer or
environment details are very hard to triage.
-->

### Before filing

- [ ] I searched the [existing issues](https://github.com/boostorg/graph/issues?q=is%3Aissue) and did not find a duplicate.
- [ ] I have reproduced the bug against the `develop` branch or the latest Boost release.
- [ ] I have a minimal reproducer (or I will paste my full failing code below).

### Boost version

<!--
You can find the version number in `<boost/version.hpp>`, or by running
`git log -1` in your local Boost.Graph checkout.
-->

### Compiler family

- [ ] GCC / g++
- [ ] Clang / clang++
- [ ] MSVC (Visual Studio)
- [ ] Intel / oneAPI
- [ ] Other (specify below)

Exact compiler version: <!-- e.g. g++ 13.2.0, clang 17.0.6, MSVC 19.39 -->

### Standard library

- [ ] libstdc++
- [ ] libc++
- [ ] MSVC STL
- [ ] Other (specify below)

### Operating system

- [ ] Linux
- [ ] macOS
- [ ] Windows
- [ ] Other (specify below)

### Kind of component affected

- [ ] Graph data structure (e.g. `adjacency_list`, `adjacency_matrix`)
- [ ] Algorithm (e.g. `dijkstra_shortest_paths`, `breadth_first_search`)
- [ ] Property map
- [ ] I/O or file-format reader (e.g. `read_graphml`, `read_graphviz`)
- [ ] Visitor / event hooks
- [ ] Other (specify below)

Exact name of the affected component: <!-- e.g. dijkstra_shortest_paths -->

### Steps to reproduce

<!--
A minimal compiling program that demonstrates the problem is ideal.
A [Compiler Explorer](https://godbolt.org/z/9Esszr9Ga) link is even better.
-->

### Expected behavior

### Actual behavior

<!--
Include compiler error messages or runtime output verbatim, with file and
line numbers where applicable.
-->

### Are you willing to help?

- [ ] I'd like to submit a fix as a pull request.
- [ ] I can help diagnose or test a candidate fix.
- [ ] I'm only reporting the issue.
