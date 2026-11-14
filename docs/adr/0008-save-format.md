# ADR-0008: Save Format — Versioned Binary, String-ID Content References

**Status:** Accepted

## Context
Saves must survive content additions. Numeric content indices break the moment a
modder or a version bump reorders a TOML file. Saves must also be compact and
fast to load.

## Decision
- Save files are **versioned binary**, little-endian.
- Header: magic, format version, `GameConfig`, master seed, active RNG state.
- Body: `entt::registry` snapshot, world state, pending `Command`s, per-player
  capability and design-progress state.
- **Content is referenced by stable string IDs** (`module.bronze_sword`,
  `tech.steam_power`), never numeric indices.
- Replays = `(seed, GameConfig, command log)`; no registry snapshots needed.
- Extension: `.asterism`.

## Consequences
+ Saves survive content additions and content reordering.
+ Replays are tiny — just the command stream + seed + config.
+ A content-reload or version-up tool can migrate saves without re-encoding
  whole registries.
- String IDs in saves use more bytes than indices — accepted for clarity.
- Forward compatibility for save-breaking changes requires an explicit migration
  step per format version bump.
  