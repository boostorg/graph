# Contributing to Boost.Graph

## We are welcoming contributions

- Bug fixes and test cases for them
- New algorithms (with tests, documentation, complexity notes, and a reference)
- Documentation, examples, performance work
- Concept refinements (handle with care as these are API-visible)

## Maintainers

| Maintainer | Role | Focus | Availability | Contact |
|---|---|---|---|---|
| **Jeremy W. Murphy** | Principal maintainer. Holds merge authority and final say on design decisions. | Algorithms, technical review, library architecture. 10+ years on BGL. | Part-time, best-effort. | `jeremy.william.murphy -at- gmail.com` |
| **Arnaud Becheler** | Assistant maintainer. Triage, review, and contributor support; defers to Jeremy on merge decisions. | Documentation, modernization, contributor onboarding. | Full-time, funded by the [C++ Alliance](https://cppalliance.org) to assist Jeremy. | `arnaud.becheler -at- gmail.com` |

The authoritative maintainer list lives in [meta/libraries.json](meta/libraries.json); keep this section in sync with it. Maintainers aim to provide first-pass review on new PRs within two weeks.

## Getting set up

Clone the [Boost superproject](https://github.com/boostorg/boost):

```bash
git clone --depth 1 https://github.com/boostorg/boost
cd boost
git submodule update --init --depth 1
./bootstrap.sh
./b2 headers
```

Then replace `libs/graph/` with your fork:

```bash
rm -rf libs/graph
git clone https://github.com/<you>/graph libs/graph
cd libs/graph
git remote add upstream https://github.com/boostorg/graph
```

## Building and testing

- Headers only: `./b2 headers` from boost root
- Compiled components: `./b2` from `libs/graph`
- All tests: `./b2` from `libs/graph/test` (takes ~10 min)
- Single test: `./b2 cycle_canceling_test` from `libs/graph/test`
- Different C++ standard: `./b2 cxxstd=20`
- Different compiler: `./b2 toolset=clang`

### Naming conventions

BGL follows the standard Boost / STL conventions:

| Kind | Style | Example |
|---|---|---|
| Functions, types, variables, files | `snake_case` | `dijkstra_visitor`, `add_edge`, `adjacency_list.hpp` |
| Concept names | `PascalCase` | `IncidenceGraph`, `VertexListGraph` |
| Template parameters | `PascalCase` | `template <class Graph, class WeightMap>` |
| Tag-dispatch types | `snake_case` + `S` suffix | `vecS`, `directedS` |
| Macros | `BOOST_GRAPH_` prefix, `SCREAMING_SNAKE_CASE` | `BOOST_GRAPH_DECLARE_EDGE_PROPERTY` |

## Using other Boost libraries

When writing new code:

- **Prefer `std::`** when the supported C++ standard has an equivalent. Use `std::function`, `std::shared_ptr`, `std::tuple` over their Boost counterparts.
- **Avoid pulling in new heavy dependencies** (Spirit, property_tree, xpressive, etc.) without justification: BGL is gradually migrating away from heavy Boost dependencies. Open a discussion before adding one.
- **Concept checks** (`boost::concept`) are encouraged for any new public template. New public APIs should respect the existing concept hierarchy. 
- Track BGL's position in the Boost dependency DAG via [boostdep](https://github.com/boostorg/boostdep)'s hosted reports. The long-term goal is to reduce BGL's level over time:
    - **[Module levels](https://pdimov.github.io/boostdep-report/master/module-levels.html#graph)** shows BGL's depth in the global DAG. A rising level means new transitive Boost dependencies have been pulled in.
    - **[Per-library page](https://pdimov.github.io/boostdep-report/develop/graph.html)** shows BGL's direct deps and per-dep `#include` counts.

## Pull request process

- Fork, branch from `develop`, PR back to `develop`
- One logical change per PR; rebase before requesting review
- Tests required for new features and bug fixes
- **Open (non-draft) PRs are assumed ready for review.** Use GitHub's Draft state while iterating, then mark the PR as *Ready for review* when you want maintainers to look at it.
- A maintainer will review within ~2 weeks (see Maintainers below)
- Squash on merge by default

## Merge criteria

Before a PR is merged, all of the following must hold:

1. CI is green on the full matrix (gcc-14 + clang-19, C++14/17/20/23).
2. New behavior has tests.
3. Bug fixes have a regression test.
4. Documentation under [doc/](doc/) is updated if the public API changed.
5. No new compiler warnings on the supported toolchains.
6. The PR is rebased on current `develop` with a clean commit history.
7. The principal maintainer has approved.

## Reporting bugs

1. Search [existing issues](https://github.com/boostorg/graph/issues) first
2. Note compiler, version, OS, Boost version
3. Minimal reproducer on [Compiler Explorer](https://godbolt.org/z/37dPWd5bs)
4. Expected Output versus Actual Output is explained.

## Security

See [SECURITY.md](SECURITY.md).

## License

By contributing, you agree your contribution is licensed under the [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt).
