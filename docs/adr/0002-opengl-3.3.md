# ADR-0002: Rendering Backend — OpenGL 3.3 core behind IRenderer

**Status:** Accepted

## Context
The game is 2D: hex tiles, sprites, text, fog overlays, ImGui. It will never stress
a modern GPU. Cross-platform support spans Windows, Linux, and macOS. macOS caps at
GL 4.1 core but supports 3.3 fully on every Mac since ~2012.

## Decision
- Backend: **OpenGL 3.3 core**.
- All renderer calls go through an `IRenderer` interface.
- ImGui uses the standard `imgui_impl_sdl3` + `imgui_impl_opengl3` backends.
- Loader: glad (via vcpkg).

## Rationale
- Highest common baseline across the three targets; no SDK install on Windows.
- No Vulkan/Metal/D3D12 matrix to maintain for a 2D game.
- 2D sprite batching + a few framebuffers fit comfortably in GL 3.3.
- RenderDoc works out of the box; documentation is abundant.
- `IRenderer` gives us an escape hatch: an `SdlGpuRenderer` or `BgfxRenderer`
  can be added later without touching simulation or UI code.

## Consequences
+ Minimal build and setup overhead; well-understood failure modes.
+ Team can use the most battle-tested ImGui backends.
- We accept that future Apple Silicon features (Metal-only paths) may require a
  second backend behind `IRenderer`. Not needed for v1.
  