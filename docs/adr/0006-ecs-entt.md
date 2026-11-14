# ADR-0006: Simulation Uses entt — Components Data-Only, Rules in Systems

**Status:** Accepted

## Context
We need cheap iteration over many entities (tiles, units, cities) and a
serialization-friendly layout. "ECS purism" that moves all logic into components
makes Civilization-style rules unreadable.

## Decision
- Use **entt** as the ECS.
- **Components are data-only.** No methods with behavior, no event handlers.
- **Systems own all rules.** Systems are plain functions or small classes operating
  over `entt::registry` views.
- Large, cohesive rules (research, combat, blueprint design) may live in dedicated
  modules rather than individual systems, but they still read/write components and
  never touch rendering or UI.

## Consequences
+ Component layout is serialization-friendly and cache-friendly.
+ Cheap iteration over 65k tiles × many units.
+ Rules are readable in one place per system.
- Requires discipline to prevent logic leaking into components over time.
- Some future refactor may be needed if a "component" grows into a de-facto
  aggregate. Mitigation: code review checklist.
  