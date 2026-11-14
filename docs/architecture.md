# Asterism — Seed Architecture Document

**Status:** v1.0 — architecture locked, ready for implementation
**Project name:** Asterism · **Namespace:** `ast` · **CMake prefix:** `ast_*` · **Save extension:** `.asterism`
**Target:** Cross-platform desktop (Windows / Linux / macOS), C++23, ImGui UI
**Document purpose:** Seed architecture — the shape of the codebase, the decisions taken, and the contracts between modules.

---

## 1. Goals & Non-Goals

### v1 Goals
- 2D hex-grid, turn-based 4X game.
- Cross-platform desktop build (Win/Linux/macOS).
- Deterministic core simulation (fixed-point, seeded RNG).
- **Deterministic unit constructor:** player-authored blueprints, produced by engineer capability.
- **Unit experience:** capped, diminishing-returns veteran progression; workers excluded.
- Save/load, replays, headless CI mode.
- Data-driven content in TOML behind an abstracted loader.

### v1 Non-Goals
- Multiplayer, modding runtime, scripting runtime, 3D.
- Isometric projection (top-down orthographic).
- Culture / Religious / Diplomatic victories.
- Content hot-reload (interface present, disabled).
- Worker experience.
- Blueprint folder UI (deferred until needed).

---

## 2. High-Level Architecture

```
+--------------------------------------------------------------+
|                         Application                          |
|  +-----------+   +-----------+   +-----------+   +---------+ |
|  |  Platform |   |  Renderer |   |    UI     |   |  Audio  | |
|  |  (SDL3)   |   | (GL 3.3)  |   | (ImGui)   |   |(miniaudio)| |
|  +-----+-----+   +-----+-----+   +-----+-----+   +----+----+ |
|        +-------+-------+-------+-------+--------------+      |
|                        |                                     |
|                  +-----v------+                              |
|                  |  Engine    |                              |
|                  +-----+------+                              |
|                        |                                     |
|        +---------------+----------------+                    |
|        |                                |                    |
|  +-----v------+                  +------v-------+            |
|  | Simulation |  <--- Commands --| GameSession  |            |
|  |  (entt +   |  --- GameView -->| (glue/state) |            |
|  |  fixed)    |                  +------+-------+            |
|  +-----+------+                         |                    |
|        |                                |                    |
|  +-----v------+                  +------v-------+            |
|  |  Content   |                  |   Commands   |            |
|  |  (TOML)    |                  |  (intents)   |            |
|  +------------+                  +--------------+            |
+--------------------------------------------------------------+
```

**Dependency direction:** strictly top → bottom. The simulation never links UI, renderer, platform, or audio.

---

## 3. Technology Stack

| Concern | Choice | Notes |
|---|---|---|
| Language | C++23 | `std::expected`, `std::format`, ranges, `std::span`, `std::mdspan`, `[[nodiscard]]` |
| Build | CMake ≥ 3.28 + Ninja + `CMakePresets.json` | |
| Dependency mgmt | **Hybrid** (see ADR-01) | vcpkg + FetchContent pinned by commit hash |
| Window/Input | SDL3 | via vcpkg |
| Rendering | OpenGL 3.3 core behind `IRenderer` | see ADR-02 |
| UI | Dear ImGui (docking branch) | FetchContent, pinned |
| Text | `stb_truetype` | |
| Audio | miniaudio | |
| ECS | `entt` | FetchContent, pinned |
| Content format | TOML via `toml++` behind `IContentProvider` | FetchContent, pinned |
| Save format | Versioned binary, little-endian | Content refs by string ID |
| Numeric | `Fixed<int64_t, 1024>` (Q54.10) for gameplay; `float` for presentation only | see ADR-03 |
| Grid | Hex, axial `(q, r)`, pointy-top, top-down orthographic | see ADR-04 |
| Tests | Catch2 v3 | FetchContent, pinned |
| Logging | `spdlog` | FetchContent, pinned |

---

## 4. Architectural Decision Records

### ADR-01 — Dependency management: hybrid, pinned by commit hash

Two tiers:

- **vcpkg (manifest mode):** system-level or non-trivial builds — SDL3, OpenGL loader.
- **CMake FetchContent, pinned to 40-char commit hashes:** pure-C++ / header-only — `entt`, `toml++`, `imgui`, `stb`, `miniaudio`, `spdlog`, `Catch2`.

```cmake
FetchContent_Declare(entt
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG        <40-char-commit-hash>   # tag only in comment
)
```

**Rules:**
- Pin to commit hashes, never to tags (tags can be reassigned).
- Cache `_deps/` in CI; local builds benefit from ccache/sccache.
- Promote a FetchContent dependency to a vcpkg overlay port only if it grows build complexity.

**Cost:** slightly slower clean configure/build, network required on first configure. Accepted.

---

### ADR-02 — Rendering: OpenGL 3.3 core behind `IRenderer`

Rationale:
- Highest common baseline on Windows (no SDK), Linux, and macOS (which caps at GL 4.1 but supports 3.3 fully).
- No Vulkan/Metal/D3D12 matrix for a 2D game.
- `imgui_impl_opengl3` + `imgui_impl_sdl3` are the most battle-tested ImGui backends.
- 2D sprite batching + a few framebuffers fit comfortably within GL 3.3.
- RenderDoc-friendly, well-documented.
- **Escape hatch:** all rendering goes through `IRenderer`; an `SdlGpuRenderer` or `BgfxRenderer` can be swapped in at boot later.

---

### ADR-03 — Fixed-point math for all gameplay

- `Fixed<int64_t, 1024>` (Q54.10) for every gameplay-affecting value (HP, attack, production, growth, capability, XP, etc.).
- `float` only for presentation (camera, UI layout, animation).
- Conversions happen at module boundaries only; a single formula never mixes the two.
- Guarantees bit-identical simulation state across compilers and platforms. Makes replays and future multiplayer safe.

---

### ADR-04 — Hex grid, axial coordinates, top-down orthographic

- Storage: `(q, r)` axial, flat array indexed `q + r * width`.
- Layout: pointy-top, horizontal.
- `HexCoord` and `HexLayout` encapsulate conversions, distance, neighbors, line, ring, spiral, and A*.
- Renderer draws with top-down orthographic projection; no isometric transform.

---

### ADR-05 — Map size as a runtime parameter

- `GameConfig { width, height, seed, ... }`, validated `2 ≤ w, h ≤ 256`.
- `World` allocates a flat `std::vector<Tile>`; `std::mdspan` provides multi-dim indexing.
- **No compile-time dimensions anywhere.** Pathfinding, AI, fog-of-war, and saves scale with the runtime parameter.
- Allocations for a 256×256 map are on the order of a few MB — no streaming needed for v1.

---

### ADR-06 — Simulation on `entt`, rules in systems

- Components are **data only**. All rules live in systems or free functions.
- Rationale: cheap iteration over tiles × units, serialization-friendly layout, headless simulation, readable complex rules (avoiding "ECS purism" that turns Civ rules into unreadable spaghetti).

---

### ADR-07 — Command-based UI interaction

- ImGui reads a read-only `GameView` snapshot.
- Every user action becomes a `Command` enqueued to the `CommandQueue`.
- `GameSession::step()` applies commands. **The UI never mutates the registry.**
- Enables replay, testing, and future multiplayer without refactoring.

---

### ADR-08 — Save format: versioned binary, string-ID content references

- Header: magic, format version, `GameConfig`, seeds.
- Body: registry snapshot, world snapshot, RNG state, pending commands.
- Content referenced by **stable string IDs** (`module.bronze_sword`), not numeric indices — content additions don't break saves.
- Replays = `(seed, GameConfig, command log)`.

---

### ADR-09 — Unit constructor: deterministic, engineer-driven, no randomness

**Model:**
1. Content DB declares **chassis** (`humanoid`, `cavalry`, `siege`, …) and **modules** (`bronze_sword`, `armor`, `spike`, `steam_engine`, `wheels`, `digger`, `wings`, `gasoline_engine`, …).
2. Each chassis has base stats, a **fixed number of module slots**, and compatibility constraints.
3. Modules are unlocked by techs. Each contributes fixed deltas (attack, defense, move, specials).
4. Player opens `BlueprintEditorPanel`, chooses chassis + modules from unlocked options, previews derived stats, submits `OrderDesignCommand`.
5. A `DesignProject` enters the player's `DesignQueue` and consumes engineer capability per turn, analogous to research.
6. When progress reaches 100%, a player-scoped `Blueprint` entity is created and the resulting unit becomes buildable in that player's cities.

**Design rules (locked):**
- **No randomness.** Outcome is a pure function of `(composition, cost, engineer capability, turns elapsed)`.
- **Tech gates everything.** Cannot slot an unlocked module.
- **Blueprints are per-player** and persistent within a game.
- **Progress is preserved per project**, like science research — switching focus and returning resumes where it left off.
- **Blueprint effects are static** in v1 (no refits/upgrades).
- **Experience attaches to unit instances**, not blueprints — two units built from the same blueprint diverge in veteran level.
- **No hard cap** on blueprints per player. If the build-order UI becomes unwieldy, introduce **folders** in the build list (post-v1).

**Engineer capability (locked):**
- Capability is produced **per city** by buildings/specialists (e.g., a library improves science velocity; a workshop improves engineer throughput) and **aggregated player-wide**.
- `CapabilitySystem` maintains two capability kinds: **Scientist** and **Engineer**.
- `ResearchSystem` consumes Scientist capability; `DesignQueueSystem` consumes Engineer capability.
- Symmetric design keeps room for future capability kinds (e.g., Diplomat) without refactoring.

**Cancellation policy (locked):** mirrors science research — accumulated progress on a `DesignProject` is preserved and resumes when the project is re-selected. No gold refund path in v1.

---

### ADR-10 — Unit experience: capped, diminishing returns, lost on death

- Veteran levels (working set): `Recruit → Veteran → Elite → Legendary` (3 promotion steps).
- Each level grants a small fixed bonus (e.g., +10% attack at Veteran, +1 move at Elite). Total ceiling is deliberately modest.
- XP gain diminishes per level; thresholds grow sharply.
- **All XP is lost on unit death.** Rebuilt units start as Recruit.
- `ExperienceSystem` owns gain triggers (combat, movement), thresholds, bonus application, and death-clearing.
- **Workers are excluded from experience in v1** — avoids degenerate "farm proficiency" behavior and keeps the worker loop simple.

---

### ADR-11 — v1 victory types: Domination, Science, Score

- **Domination** — validates combat and conquest loop.
- **Science** — validates the tech tree and research velocity.
- **Score** — fallback and time-limited end.
- `GameConfig.victoryMask` records which types were enabled. Culture/Religious/Diplomatic hooks exist in `victories.toml` but no systems are active in v1 — adding them post-v1 does not break saves.

---

### ADR-12 — Project name: Asterism

- One word, neutral, no known trademark collision in the game category.
- Namespace `ast`, target prefix `ast_*`, save extension `.asterism`, window title "Asterism".

---

## 5. Module Breakdown

### 5.1 `common/`
- `math`: `Vec2`, `Vec3` (float, presentation), `Fixed`, `HexCoord`, `HexLayout`.
- `ids`: `EntityId` (`entt::entity`), `TileId`, `PlayerId`, `ContentId` (interned string), `BlueprintId`.
- `result`: `Result<T> = std::expected<T, Error>`.
- `rng`: PCG32; derived seeds `hash(masterSeed, turn, entity, purpose)`.
- `log`, `assert`, `config`.

### 5.2 `platform/`
- `IWindow`, `IInput`, `IFilesystem`, `ITimer`, `IDialogs`.
- `Sdl3Platform` implementation.

### 5.3 `renderer/`
- `IRenderer`, `GlRenderer`.
- `SpriteBatch`, `TextureAtlas`, `Camera2D`, `TextRenderer`.
- `HexGridRenderer`, `FogRenderer`, `BorderRenderer`, `DebugDraw`.

### 5.4 `ui/`
- `UiContext`, `Screen` (`MainMenu`, `NewGame`, `InGame`, `Settings`, `LoadGame`).
- Panels: `MapPanel`, `UnitPanel`, `CityPanel`, `TechTreePanel`, `CivicsPanel`, `DiplomacyPanel`, `TurnPanel`, `BlueprintEditorPanel`, `DesignQueuePanel`, `CapabilityPanel`, `DebugPanel`, `ConsolePanel`.

### 5.5 `engine/`
- `Application`, `GameLoop`, `ServiceLocator`, `AssetManager`, `EventBus`, `CommandQueue`, `Command` visitor.

### 5.6 `sim/`

**Components (entt, data-only):**
- World: `TileFeature`, `ResourceNode`, `Vision`.
- Actors: `Unit`, `City`, `Owner`, `Position`.
- Progression: `Experience`, `Research`, `Production`, `Growth`, `Happiness`, `Culture`.
- Capability: `CityCapability` (per-city Scientist/Engineer production), aggregated via `PlayerCapability`.
- Constructor: `Blueprint`, `DesignProject`, `DesignQueue`.
- Diplomacy: `DiplomacyState`, `Relations`, `Treaty`.

**Systems:**
- `MovementSystem`, `PathfindingSystem`.
- `CombatSystem`, `ExperienceSystem`.
- `ProductionSystem`, `GrowthSystem`, `HappinessSystem`, `CultureSystem`.
- `ResearchSystem`, `CapabilitySystem`.
- `DesignQueueSystem`, `BlueprintSystem`.
- `DiplomacySystem`, `FogOfWarSystem`, `TurnPhaseSystem`, `VictorySystem`.

All gameplay math through `Fixed`.

### 5.7 `session/`
- `GameSession` owns registry, world, RNG, `GameConfig`.
- `GameConfig` includes `{ width, height, seed, victoryMask, ... }`.
- `SaveLoad` — versioned binary.
- `ReplayLog` — seed + config + command stream.
- `GameView` — read-only projection for UI/renderer.

### 5.8 `content/`
- `IContentProvider` → `TomlContentProvider` → `ContentDatabase`.
- Files: `units_chassis.toml`, `units_modules.toml`, `units_stats.toml`, `buildings.toml`, `techs.toml`, `terrain.toml`, `leaders.toml`, `victories.toml`, `capabilities.toml`.
- Stable string content IDs.
- Hot-reload contract: `IContentProvider` emits `ContentReloaded`; subscribers clear derived caches. Disabled in v1.

### 5.9 `tests/`
- Unit tests per system; fixed-point math tests; content validation.
- Determinism test: turn-by-turn state hash across seeds and configs.
- Golden save/replay round-trip.
- Headless replay CI (`ast_app --headless --replay <file> --state-hash-out <path>`).

---

## 6. Data Flow

### Frame loop
```
loop:
  dt = timer.tick()
  input.poll()
  commands = ui.collect()          # from ImGui panels
  session.enqueue(commands)
  session.step()                   # apply commands; advance turns
  view = session.build_view()
  renderer.begin_frame()
  renderer.draw_world(view)
  ui.render(view)
  renderer.end_frame()
```

### Turn flow
```
begin_turn(player)
  -> process queued commands
  -> AI decisions
  -> unit AI / city AI
  -> combat resolution
  -> capability accumulation
  -> research / design queue / production / growth / happiness / culture
  -> events (natural wonders, barbarians, etc.)
end_turn(player)
```

**Determinism contract:** identical `(masterSeed, GameConfig, commandStream)` ⇒ bit-identical state hashes across platforms.

---

## 7. Determinism & Testing

- Fixed-point everywhere in sim → no FP drift.
- Derived RNG only: `PCG32(hash(masterSeed, turn, entity, purpose))`. No system calls `rand()` or `time()`.
- Headless mode for CI.
- Cross-platform state-hash matrix in CI (Linux, Windows, macOS).
- Golden replay and save round-trip tests.
- Content validation in CI (schema, uniqueness, cross-references).

---

## 8. Directory Layout

```
/CMakeLists.txt
/CMakePresets.json
/vcpkg.json
/README.md
/docs/
    architecture.md              (this file)
    content-schema.md
    adr/
        0001-dependency-management.md
        0002-opengl-3.3.md
        0003-fixed-point.md
        0004-hex-grid.md
        0005-map-size-runtime.md
        0006-ecs-entt.md
        0007-command-ui.md
        0008-save-format.md
        0009-unit-constructor.md
        0010-unit-experience.md
        0011-victories-v1.md
        0012-project-name-asterism.md
/src/
    common/  platform/  renderer/  ui/  engine/  sim/  session/  content/
/app/
    main.cpp
/assets/
    textures/  fonts/
    data/                        (*.toml)
    maps/
/tests/
    unit/  integration/  determinism/
```

---

## 9. Milestones

1. **M0 — Skeleton:** CMake presets, vcpkg manifest, FetchContent pins, SDL3 + GL 3.3 + ImGui dockable shell, CI builds green on 3 platforms.
2. **M1 — Map:** hex generation at runtime dims, terrain from TOML, camera, tile selection, camera pan/zoom.
3. **M2 — Units:** entt units, pathfinding, fog of war, basic veteran levels.
4. **M3 — Turn loop:** command queue, turn phases, end turn, headless replay harness + state-hash CI.
5. **M4 — Cities:** founding, tile ownership, production, growth, happiness, capability sources (libraries, workshops, specialists).
6. **M5 — Tech & Constructor:** tech tree, module unlocks, blueprint editor, design queue, engineer capability, capability panel.
7. **M6 — Combat & Diplomacy:** combat with veteran bonuses, minimal diplomacy stubs (war/peace only).
8. **M7 — Save/Load + Replays:** versioned binary format, golden tests, determinism matrix.
9. **M8 — Victories & Polish:** Domination, Science, Score; AI tuning; UI polish.

---

## 10. Content Schema Sketch

Minimal example to anchor the schema (full schema in `content-schema.md`):

```toml
# units_chassis.toml
[humanoid]
display_name = "Humanoid"
base_hp      = 100
base_move    = 2
base_cost    = 40
slots        = 3
tags         = ["infantry"]

[cavalry]
display_name = "Cavalry"
base_hp      = 90
base_move    = 4
base_cost    = 60
slots        = 2
tags         = ["mounted"]

# units_modules.toml
[bronze_sword]
display_name = "Bronze Sword"
requires_tech = "tech.bronze_working"
effects      = { attack = 8 }

[armor]
display_name = "Armor"
requires_tech = "tech.bronze_working"
effects      = { defense = 6, move = -1 }

[steam_engine]
display_name = "Steam Engine"
requires_tech = "tech.steam_power"
allowed_chassis = ["siege"]
effects      = { move = 2 }

# capabilities.toml
[scientist_basic]
display_name = "Scientist"
kind         = "scientist"
rate         = 2

[engineer_basic]
display_name = "Engineer"
kind         = "engineer"
rate         = 2
```

---

## 11. Open Implementation Questions (not architectural blockers)

These can be decided during implementation without changing the architecture shape:

1. **Diagram rendering fidelity** — sprite-sheet resolution and atlas packing strategy (per-frame vs. packed). Choose during art pipeline setup.
2. **AI implementation approach** — rule-based utility scoring vs. lightweight behavior tree. Both fit the existing `Command` + system model. Decide at M8.
3. **Camera edge-scroll vs. drag-scroll default** — a UI preference; both implemented, default chosen after playtesting.
4. **Exact engineer/scientist capability numbers** — balance data; tuned in `capabilities.toml` without code changes.
5. **Combat formula shape** — RNG-driven damage rolls via derived PCG32 vs. deterministic ratio-based combat. Both supported by the current RNG contract; choose during M6 based on playtest feel.

---

## 12. Locked Summary

| Decision | Value |
|---|---|
| Project name | **Asterism** |
| Namespace / target prefix | `ast` / `ast_*` |
| Language | C++23 |
| Build | CMake + Ninja + presets |
| Dependency mgmt | vcpkg + FetchContent pinned by commit hash |
| Window/Input | SDL3 |
| Rendering | OpenGL 3.3 core behind `IRenderer` |
| UI | Dear ImGui (docking) |
| ECS | `entt` |
| Content | TOML behind `IContentProvider` |
| Save | Versioned binary, string-ID content refs |
| Numeric | `Fixed<int64_t, 1024>` for gameplay; float only in presentation |
| Grid | Hex axial, pointy-top, top-down orthographic |
| Map size | Runtime parameter, `2..=256` per side |
| Unit constructor | Deterministic, engineer-capability-driven, no randomness, per-player blueprints, unlimited count |
| Engineer capability | City-level pool, player-aggregated; same model as scientists |
| Design cancellation | Research-like (progress preserved per project) |
| Unit experience | 3 promotion levels, diminishing returns, lost on death; workers excluded |
| v1 victories | Domination, Science, Score |
| Hot-reload | Interface present, disabled |
| Multiplayer / modding / scripting | Post-v1 |
| Worker experience | Excluded from v1 |
| Blueprint folders | Deferred until build UI becomes unwieldy |
