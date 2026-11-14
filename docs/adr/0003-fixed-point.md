# ADR-0003: Fixed-Point Math for Gameplay

**Status:** Accepted

## Context
Replays and CI determinism require bit-identical simulation state across compilers
and platforms. Floating-point arithmetic in C++ is not guaranteed to produce
identical results across optimization levels or CPU architectures.

## Decision
- Gameplay numeric type: `Fixed<int64_t, 1024>` (Q54.10 — 10 fractional bits).
- All state-mutating gameplay math uses `Fixed` (HP, attack, defense, move,
  production, growth, capability, experience, costs).
- `float` is permitted only in presentation: rendering, camera, UI layout, animation.
- Conversions between `Fixed` and `float` happen at module boundaries only.
- A single formula must never mix `Fixed` and `float` intermediates.

## Consequences
+ Bit-identical simulation across Win/Linux/macOS and across compilers.
+ Replays are safe. Future multiplayer is safe.
+ A single `Fixed` type keeps arithmetic readable.
- Formulas must be authored in fixed-point units; authoring tools and content
  files document scale (1 unit = 1.0, so `0.5` is `512` in `Fixed` but `0.5` in TOML).
- Sub-1/1024 precision is impossible (accepted per architecture decision).
