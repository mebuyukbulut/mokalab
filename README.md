<p align="center">
  <img src="https://github.com/user-attachments/assets/51b6f5cf-358a-4a42-8a2e-991aa22295a9" width="100%">
</p>

# MokaLab — Real-Time Graphics Editor & Rendering Sandbox

> Built from scratch in Modern C++ and OpenGL. No engine. No shortcuts.

## Demo Video
[![Demo Video](https://img.youtube.com/vi/fZnJDtRmH1w/maxresdefault.jpg)](https://www.youtube.com/watch?v=fZnJDtRmH1w)

---

## Overview

MokaLab is a real-time 3D graphics editor and rendering sandbox developed independently from the ground up. The goal is not to replicate existing tools, but to deeply understand rendering systems, engine architecture, and graphics tooling by building them from scratch.

**Stack:** C++, OpenGL, GLSL, ImGui, CMake, YAML

---

## Architecture

### Multi-Pass Rendering Pipeline

The renderer is built around a sequential pass system, each writing to dedicated render targets:

| Pass | Description |
|---|---|
| Shadow Pass | Directional light shadow map generation via depth-only render target |
| Material Pass | PBR shading with shadow map sampling |
| Matcap Pass | Normal-based spherical environment mapping for debug/preview |
| Wireframe Pass | `GL_LINE` polygon mode overlay |
| Background Pass | Skybox / background rendering |
| Grid Pass | World-space infinite grid with depth-correct rendering |
| Light Pass | Gizmo rendering for directional, point, and spot lights |
| Selection Pass | GPU color-ID picking via offscreen integer render target |
| Outline Pass | Post-selection silhouette highlighting |
| Post-Process Pass | Composable fullscreen shader passes via FX stack |

### Render Targets

Three separate framebuffer classes manage rendering output:

- **ColorRenderTarget** — color + depth for main scene and post-processing ping-pong
- **ShadowMapTarget** — depth-only for shadow map generation  
- **SelectionRenderTarget** — integer color ID + depth for GPU picking

### GPU Color-ID Picking

Entity selection is handled entirely on the GPU. Each entity is assigned a unique integer ID rendered into an offscreen `SelectionRenderTarget`. On mouse interaction, the pixel at the cursor position is read back to identify the selected entity — supports both single-click and drag-box multi-selection.

### Uniform Buffer Objects (UBO)

Per-frame GPU data (view matrix, projection matrix, camera position) is uploaded once per frame via a `FrameUniforms` UBO bound to binding point 0. All shaders that need per-frame data share this buffer — no redundant uniform uploads across shader programs.

### Post-Processing FX Stack

A data-driven, composable post-processing system:

- Effects are defined declaratively via `FXInstanceDefinition` — shader paths, parameter definitions, labels, tooltips
- Parameters are typed via `std::variant<float, int, bool, vec2, vec3, vec4>` — no void pointer casting, no runtime type guessing
- Active effects are composed in a ping-pong loop between two `ColorRenderTarget` buffers
- Effects can be toggled per-instance; opacity is adjustable at runtime
- Built-in effects: Grayscale, Invert, Sepia, Vignette, Gamma Correction, Posterize, Pixelate

### Entity-Component Architecture

Scene objects are built from components attached to entities:

- **Transform** — hierarchical parent-child relationships, local/world space decomposition
- **RenderComponent** — decoupled from transform; references model and material
- **LightComponent** — directional, point, and spot light variants with shadow mapping integration

### Event Dispatcher

A type-safe publish-subscribe system decouples editor UI from engine internals. Systems subscribe to `EventType` enum variants with `std::function` callbacks. No direct coupling between subsystems — UI dispatches events, engine systems respond.

```cpp
dispatcher.subscribe(EventType::Select, [&](std::unique_ptr<EventData> data) {
    // handle selection without knowing who triggered it
});
```

### Asset Management & Async Loading

An `AssetManager` handles resource lifetime and de-duplication. Large mesh imports are handled asynchronously to avoid frame stalls — assets are loaded off the main thread and made available to the renderer once ready.

### YAML Scene Serialization

Full scene state — entities, components, hierarchy, material references — is serialized to and from YAML. Scenes persist across sessions with complete fidelity.

### Particle System

A CPU-side particle system with:

- Parallel update via `std::execution::par` — ~3x faster than sequential on multi-core
- Pluggable force system (`IForce` interface) — gravity, custom forces composable at runtime
- Pluggable emitter shapes (`IEmitterShape`) — point, sphere, and custom shapes
- Pluggable color providers (`IColorProvider`) — constant or animated color over lifetime
- Up to 10,000 active particles with spawn rate control

### Debug Visualization

- Wireframe mode
- Matcap shading mode
- Grid rendering
- Light gizmos (per light type)
- Selection highlighting / outline pass
- ImGui inspector for all scene and rendering parameters

---

## Features Summary

- [x] PBR shading pipeline (albedo, metallic, roughness, emissive)
- [x] Shadow mapping (directional light)
- [x] Multi-pass render architecture
- [x] GPU color-ID entity picking (single + box select)
- [x] Selection outline rendering
- [x] Post-processing FX stack (composable, data-driven)
- [x] Matcap / wireframe debug views
- [x] UBO-based per-frame GPU data sharing
- [x] Async asset loading
- [x] Entity-component architecture with hierarchical transforms
- [x] Directional, point, and spot lights with gizmos
- [x] YAML scene serialization
- [x] CPU particle system with parallel update
- [x] Event dispatcher (decoupled pub-sub)
- [x] ImGui-based editor UI (scene tree, inspector, debug window)
- [x] Virtual asset path system for built-in primitives
- [x] CMake build system

---

## Roadmap

- [ ] Deferred rendering
- [ ] Bloom post-processing
- [ ] Scripting system
- [ ] Animation system
- [ ] HalfEdge mesh structure and mesh editing
- [ ] Vulkan backend

---

## Screenshots

<p align="center">
  <img src="https://github.com/user-attachments/assets/d46d91df-f36f-4a5e-8cc3-d3531cbb4c14" width="48%">
  <img src="https://github.com/user-attachments/assets/774fb1e0-7a72-4867-93ba-14d74131beb1" width="48%">
</p>

---

## Building

```bash
git clone https://github.com/mebuyukbulut/mokalab.git
cd mokalab
cmake -B build
cmake --build build
```

**Dependencies** (included via `external/`): glad, GLFW, glm, ImGui, stb, yaml-cpp

---

## Philosophy

MokaLab is not a production engine. It is a graphics laboratory — a place to understand rendering systems by building them, not wrapping them.

Everything here is written by hand: the render targets, the FX system, the event dispatcher, the asset manager, the particle system, the HalfEdge structure. The goal is always to understand, not just to ship.
