# ADR-0012: Project Name — Asterism

**Status:** Accepted

## Context
The working title "Civilization" is trademarked. Earlier candidates were rejected:
"Hegemony" has a similar-named game, and "Humanity" is actively trademarked
(tha ltd, game category).

## Decision
- Project name: **Asterism** (an optical phenomenon; also a constellation pattern).
- Namespace: `ast` (e.g., `ast::sim`, `ast::ui`).
- CMake target prefix: `ast_*` (e.g., `ast_common`, `ast_sim`).
- Save extension: `.asterism`.
- Window title: "Asterism".

## Consequences
+ Neutral, one word, no known collision in the game category.
+ Easy to grep; visually distinct in binary strings.
- May need a logo design pass at branding time.
