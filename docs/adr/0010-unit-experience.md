# ADR-0010: Unit Experience — 3 Levels, Diminishing Returns, Lost on Death

**Status:** Accepted

## Context
A veteran system adds narrative to units and rewards careful play. It also
risks snowballing — a maxed unit that cannot be countered collapses the
strategic space.

## Decision
- Veteran levels: **Recruit → Veteran → Elite** (3 promotions).
- Each promotion grants a small fixed bonus; total ceiling is deliberately modest
  (e.g., +10% attack per level, +1 move at Elite — tuned in TOML).
- XP gain is **diminishing**: thresholds grow sharply per level.
- **All XP is lost on unit death.** Rebuilt units restart as Recruit.
- `ExperienceSystem` owns gain triggers, thresholds, bonus application, and
  death-clearing.
- **Workers are excluded from experience in v1** — avoids degenerate
  "farm proficiency by re-issuing build orders" behavior.

## Consequences
+ Veterans are meaningful but not snowbally.
+ Losing a veteran is a real loss, keeping combat tense.
- No worker progression in v1 (accepted).
- The XP curve and per-level bonuses need playtesting (M6/M8).
