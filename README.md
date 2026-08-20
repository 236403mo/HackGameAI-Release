# HackGameAI

A small top-down action game built in **Unreal Engine 5.8**, with gameplay inspired by the abstract hacking minigame sequences in *NieR: Automata*.

The game itself is deliberately small. Its primary purpose is to serve as a working demonstration of an **AI-assisted, agentic Unreal Engine development workflow** — building a real, playable UE project using [Claude Code](https://claude.com/claude-code) driving the Unreal Editor over MCP, with role-based specialist agents and human review at each step.

> This project is not affiliated with, endorsed by, or connected to Square Enix or PlatinumGames. It contains no assets, layouts, UI, or audio from *NieR: Automata*.

## Why it exists

Most demonstrations of AI-assisted game development stop at a code snippet. This repository is the opposite: a complete, buildable UE 5.8 project with three playable missions, where the interesting artifact is as much the **process** as the game.

The process itself is documented in the repository:

| File | What it covers |
|---|---|
| [`CLAUDE.md`](CLAUDE.md) | The operating rules the agents work under — scope control, C++/Blueprint split, verification and acceptance gates |
| [`GAME_DESIGN.md`](GAME_DESIGN.md) | The design document the implementation was driven from |
| [`Docs/ArtDirection/README.md`](Docs/ArtDirection/README.md) | The approved visual baseline and the constraints behind it |
| [`.claude/agents/`](.claude/agents/) | The specialist agent role definitions |

## Development workflow

The project was built with:

- **Claude Code** as the development environment and coordinator,
- **Unreal Engine MCP** (the `ModelContextProtocol` plugin, which ships with UE 5.8) to inspect and modify the live editor — creating assets, placing actors, editing Blueprints, and running PIE,
- **role-based specialist agents** — a lead session delegating to Gameplay Engineer, Level Designer, QA Engineer, Architecture Reviewer, and Visual Reviewer roles,
- **a mix of agent-generated implementation and manual human work**, particularly for visual polish and art direction.

This was human-directed throughout. The agents implemented, verified, and reviewed against explicit direction and acceptance criteria — layout, visual results, and milestone acceptance were judged manually, and a number of visual decisions were made or corrected by hand. `.mcp.json` in the repository root is configured for an `unreal-mcp` HTTP server on `http://127.0.0.1:8000/mcp`.

## Gameplay

Top-down perspective with a follow camera. The player is a simple geometric pawn; enemies, walls, and objectives are readable primitives rather than detailed art.

### Controls

| Input | Action |
|---|---|
| `WASD` | Move |
| Mouse | Aim toward the cursor |
| Left Mouse Button | Fire (hold to fire continuously) |
| `Esc` | Pause (disabled on the main menu) |

Input uses Enhanced Input (`IMC_Player`, `IA_Move`, `IA_Fire`, `IA_Pause`).

### Missions

All three missions are selectable from the main menu (`L_MainMenu`) from the start — there is no unlock or save system. Each mission is a separate map, and retrying reloads it.

**Mission 1 — Navigation** (`L_Mission01`)
Introduces movement, mouse aiming, shooting, and the follow camera. The player traverses a simple walled route, destroys a stationary destructible target, and reaches the mission exit, which stays inactive until the target is destroyed. No hostile enemies and no damage source.

**Mission 2 — Stationary Threats** (`L_Mission02`)
Adds combat: stationary shooter enemies that track and fire at the player, enemy projectiles, and player health. Destructible obstacles provide cover but are not part of the win condition. The exit sits in a separate chamber behind a locked barrier that unlocks once the required shooters are destroyed.

**Mission 3 — Moving Threats** (`L_Mission03`)
Adds enemy movement, reusing the Mission 2 systems rather than replacing them. Alongside moving shooter enemies, it introduces the **Caterpillar** — a multi-segment boss whose segments follow a shared closed rectangular path at fixed offsets, moving as a connected train. Each segment is an independent actor with its own health and can be destroyed individually.

## Requirements

- **Unreal Engine 5.8**
- **Windows 64-bit** — the project targets Windows with the DX12 RHI and a Desktop hardware target
- **Visual Studio** with the *Desktop development with C++* / game development workload, to compile the C++ module

The project depends only on engine modules (`Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`) and three plugins that ship with UE 5.8 (`ModelingToolsEditorMode`, `ModelContextProtocol`, `LiveCodingToolset`). **No Marketplace/Fab content or third-party asset packs are required.**

## Getting started

1. Clone the repository.
2. Right-click `HackGameAI.uproject` and choose **Generate Visual Studio project files**. Solution files are intentionally not committed, so this step is required.
3. Build the project — either from the generated solution (`Development Editor`, `Win64`), or by opening `HackGameAI.uproject` and letting the editor rebuild the module when prompted.
4. Open `HackGameAI.uproject`. It starts on `L_MainMenu`.
5. Press Play and pick a mission.

## Audio

The sound effects used during development are **third-party assets and are intentionally not distributed in this repository.** They are excluded from version control via `.gitignore` (`/Content/HackGameAI/Sounds/`).

As a result, a fresh clone will have **unresolved sound references and will play silently.** Several Blueprints still hold references to the missing assets — these are listed in [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md). Gameplay is unaffected: the sound properties are optional and each playback call is guarded.

If you want audio, supply your own replacement sounds at the referenced paths. Please do not redistribute the excluded assets.

## Assets and provenance

The visual style relies on Unreal primitives, project-authored materials, and project-created UI rather than external art packs.

The concept and reference images under `Docs/ArtDirection/`, and some of the source textures they informed, were **generated for this project** using an AI image model; the source PNGs carry embedded C2PA Content Credentials recording that provenance. The materials, material instances, Blueprints, levels, and UI widgets were authored in-engine for this project.

## Licensing

This repository's own code and content are provided under the **MIT License** — see [`LICENSE`](LICENSE).

That license applies to the project's own work only. Specifically:

- **Third-party components** retained in this repository keep their own licenses. See [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
- **Unreal Engine, and any Epic-provided engine code, plugins, or content referenced by this project**, are governed by Epic Games' own terms, including the Unreal Engine EULA. Nothing here grants any rights over them — you obtain the engine from Epic under their terms.

Source files generated by Unreal's C++ class wizard retain Epic's default template header; this is engine boilerplate and does not change the licensing described above.

## Third-party notices

See [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) for third-party attribution, including the vendored code-review skill and the excluded audio assets.
