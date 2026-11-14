# ADR-0009: Unit Constructor — Deterministic, Engineer-Driven, No Randomness

**Status:** Accepted

## Context
The design goal is a Civilization-like 4X with a first-class unit-construction
mechanic: players author blueprints from a chassis + modules, and engineers
produce them over time. Randomness in the design process was explicitly rejected
as a source of frustration.

## Decision
- Content declares **chassis** (base stats, fixed number of module slots) and
  **modules** (deltas plus tech requirements).
- Player opens `BlueprintEditorPanel`, composes a valid blueprint from unlocked
  options, and submits `OrderDesignCommand`.
- A `DesignProject` is placed in the player's `DesignQueue`. Progress advances
  each turn based on player **engineer capability** (same model as scientists
  driving research).
- On completion, a player-scoped `Blueprint` entity is created; the unit becomes
  buildable in that player's cities.
- **Deterministic:** the completion turn is a pure function of
  `(composition cost, engineer capability, accumulated progress)`.
- **No hard cap** on blueprints per player. If the build UI becomes unwieldy,
  introduce **folders** in the build order (post-v1).
- **Cancellation mirrors research:** accumulated progress is preserved per project
  and resumes on re-selection. No gold refund.
- **Blueprint effects are static in v1** — no refits or upgrades.
- **Experience attaches to unit instances**, not to blueprints.

## Consequences
+ Player agency is high; the tech tree directly feeds meaningful choices.
+ The mechanic reuses the researcher/scientist pattern; one system, two capability
  kinds, no new subsystem family.
- Combinatorial balance is non-trivial and must be controlled via content
  (chassis slots, module cost deltas) rather than code.
- The blueprint editor UI is a non-trivial panel (M5); budget accordingly.
