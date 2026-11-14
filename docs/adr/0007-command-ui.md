# ADR-0007: UI Is Read-Only — User Actions Become Commands

**Status:** Accepted

## Context
A direct-mutation UI couples presentation to simulation and blocks replays,
determinism, testing, and future multiplayer.

## Decision
- UI reads a `GameView` — an immutable snapshot of the parts of the simulation
  the UI needs.
- Every user action is expressed as a `Command` (a variant type) enqueued to a
  `CommandQueue`.
- `GameSession::step()` applies commands to the registry. UI never touches
  the registry.
- Replays are recorded as the command stream.

## Consequences
+ Replays, determinism, testing, and eventual netcode all become tractable.
+ UI is testable in isolation against synthetic `GameView`s.
- UI must reflect state changes only via rebuilt `GameView` — occasional one-turn
  lag in the visible state is possible and must be handled in UI presentation.
- Some convenience operations (e.g., "move unit now") need to be split into
  command + view refresh.
  