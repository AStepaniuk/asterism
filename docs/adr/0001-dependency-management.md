# ADR-0001: Dependency Management — Hybrid vcpkg + FetchContent (commit-hash pinned)

**Status:** Accepted

## Context
Asterism needs SDL3 (system integration, non-trivial build), plus a set of pure-C++ or
header-only libraries (entt, imgui, toml++, spdlog, Catch2, stb, miniaudio). We want
reproducible builds across three OSes without maintaining a vcpkg fork for every
small library we touch.

## Decision
Two-tier strategy:
- **Tier 1 — vcpkg (manifest mode):** SDL3, glad. Libraries with platform-specific
  build steps or system integration.
- **Tier 2 — CMake FetchContent:** all pure-C++ / header-only libraries.
  Every dependency is pinned to a **full 40-character commit hash**, never a tag.
  Tags can be reassigned by upstream maintainers; hashes are immutable.
  The human-readable tag lives in a comment for grep-friendliness.

CI caches `_deps/` between runs. Local builds are expected to use ccache or sccache.

Audio is a soft dependency. SDL audio subsystem and miniaudio are initialised lazily, and any failure during init is logged as a warning (spdlog::warn), not a fatal error. Asterism must be fully playable without any audio device. This is required for CI (headless Linux runners have no snd_pcm), for users on broken PipeWire/PulseAudio setups, and for users who simply don't want sound. Future code that touches audio must preserve this contract.

## Consequences
+ No fork/vendoring burden for small deps; easy to patch by pointing at a fork.
+ Reproducible builds — the exact tree is byte-identical regardless of when it's cloned.
- Slightly slower clean configure/build; network required on first configure.
- Any FetchContent dependency that grows build complexity can be promoted to a
  vcpkg overlay port without changing the rest of the build.

