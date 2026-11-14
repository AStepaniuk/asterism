# ADR-0004: Hex Grid — Axial (q, r), Pointy-Top, Top-Down Orthographic

**Status:** Accepted

## Context
The game is a Civilization-like 4X. The grid type affects storage, pathfinding,
adjacency, and rendering.

## Decision
- **Hex grid**, `(q, r)` **axial coordinates**.
- Storage: flat `std::vector<Tile>` indexed `q + r * width` (with a bounding-box
  clip so `q, r` always stay inside the rectangular map).
- Orientation: **pointy-top**, horizontal layout.
- Projection: **top-down orthographic** (no isometric transform).
- `HexCoord` and `HexLayout` types live in `ast/common/math/hex.hpp` and own
  conversions, distance, neighbors, line, ring, spiral, and hex-A*.

## Consequences
+ Axial coordinates have the simplest neighbor set (6 vectors) and cheapest
  distance function (`(|dq| + |dr| + |ds|)/2`).
+ Pointy-top has more horizontal pixel real estate per hex, which reads better
  on widescreen.
+ Top-down keeps hit-testing a pure 2D problem.
- Content authors must think in axial coordinates when hand-editing maps. Mitigated
  by a future map editor (post-v1) and a TOML map format that accepts offset coords.
  