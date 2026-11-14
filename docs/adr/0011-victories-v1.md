# ADR-0011: v1 Victory Types — Domination, Science, Score

**Status:** Accepted

## Context
Six victory conditions (Domination, Science, Culture, Religious, Diplomatic, Score)
each require their own progress tracking, UI, AI evaluation, and balance work.
v1 must remain shippable before the scope is expanded laterally.

## Decision
- v1 implements **Domination**, **Science**, **Score**.
- `GameConfig.victoryMask` records which types are enabled for a given game.
- `victories.toml` contains hooks for all six; only the v1 three have active
  systems.
- Post-v1 additions (Culture / Religious / Diplomatic) do not break saves — the
  mask already records what was active when the game was created.

## Consequences
+ v1 validates combat (Domination), tech (Science), and a fallback (Score).
+ Save format is forward-compatible with post-v1 victory additions.
- Content authors writing diplomatic or cultural hooks will be unexercised in v1.
