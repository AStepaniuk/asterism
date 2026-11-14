# ADR-0005: Map Size Is a Runtime Parameter

**Status:** Accepted

## Context
Map size affects memory, pathfinding cost, AI cost, and save size. Hardcoding a
size would lock game design decisions into compile-time.

## Decision
- `GameConfig { width, height, ... }` is provided at new-game time.
- Validation: `2 ≤ width, height ≤ 256`.
- `World` allocates a flat `std::vector<Tile>` sized `width * height`.
  Access is mediated by `std::mdspan<Tile, 2>`.
- No compile-time dimension exists anywhere in the codebase.
- All systems that iterate tiles/pathfind/AI scale with the parameter.

## Consequences
+ Single binary serves small, medium, and large games.
+ Save format carries the size; replays reproduce.
- 256×256 (65,536 tiles) with several components per tile is a few MB — trivial.
  No streaming or chunking is needed for v1.
- Pathfinding perf on 256×256 must be verified at M3 (see M0 exit criteria).
