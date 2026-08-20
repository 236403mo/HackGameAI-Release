# HackGameAI — Development Rules

## Project Overview

- Project name: `HackGameAI`.
- Engine: Unreal Engine 5.8.
- Platform: Windows.
- Project type: C++ Unreal Engine project.
- The project is intentionally small and focused on AI-assisted game development.
- The main purpose is to demonstrate a practical multi-agent development workflow using Claude Code and Unreal MCP.
- The game should remain simple enough that AI agents can build, inspect, test, and iterate on it reliably.
- Use only Unreal Engine primitives, project-created materials, simple effects, and project-created UI unless the user explicitly requests otherwise.
- Do not add external asset packs or third-party gameplay dependencies unless explicitly approved.

## Core Development Goals

1. Build small, working vertical slices.
2. Keep the project easy to understand and easy to recover.
3. Prefer simple and robust implementations over clever or over-engineered ones.
4. Avoid unnecessary abstractions and speculative architecture.
5. Avoid unnecessary model calls and Unreal MCP calls.
6. Do not modify more of the project than the current task requires.
7. Preserve working functionality while adding new features.
8. Prefer incremental development that can be verified after each meaningful step.
9. Treat testing and verification as part of implementation, not as optional follow-up work.

---

## Game Scope

`HackGameAI` is a small top-down action game inspired by hacking minigames such as the hacking sequences in NieR: Automata.

The visual style intentionally relies on simple geometric shapes and clear colors rather than external art assets.

The game consists of three progressively more complex missions.

### Mission 1

Core player gameplay:

- top-down or slightly angled camera,
- player movement,
- mouse aiming,
- shooting,
- walls and arena boundaries,
- simple destructible targets or objectives,
- mission completion after the objective is satisfied.

### Mission 2

Adds stationary hostile enemies:

- stationary enemy actors,
- enemies aim or fire toward the player,
- enemy projectiles,
- player health and damage,
- enemy health and destruction,
- mission completion after all required enemies are destroyed.

### Mission 3

Adds moving hostile enemies:

- moving enemies,
- simple movement logic such as chase, strafe, patrol, or combinations of them,
- a more complex combat encounter,
- reuse of systems created for earlier missions.

Do not implement later missions unless the user explicitly requests them.

Do not create speculative systems for future missions unless doing so clearly reduces immediate duplication or is required for the current task.

---

## Content Structure

Project-specific Unreal assets should normally be organized under:

`/Game/HackGameAI`

Suggested structure:

- `/Game/HackGameAI/Core`
- `/Game/HackGameAI/Player`
- `/Game/HackGameAI/Enemies`
- `/Game/HackGameAI/Missions`
- `/Game/HackGameAI/UI`
- `/Game/HackGameAI/Materials`
- `/Game/HackGameAI/FX`

Do not create folders that are not currently needed.

---

## C++ and Blueprint Responsibilities

Use a hybrid C++ and Blueprint architecture.

### Prefer C++ for

- reusable gameplay systems,
- gameplay components,
- player and enemy state,
- weapons and projectiles,
- health and damage,
- interfaces between systems,
- shared logic used by multiple actors,
- gameplay rules,
- mission state,
- logic that is easier to test, review, and maintain as text.

Expose only the properties, functions, events, and configuration points that Blueprint actually needs.

Do not move simple one-off configuration into C++ without a clear benefit.

### Prefer Blueprint for

- actor composition,
- assigning meshes and materials,
- simple actor configuration,
- simple local behaviors,
- visual effects,
- UMG and presentation,
- level-specific setup,
- designer-facing values,
- lightweight prototyping.

Avoid large or difficult-to-follow Blueprint graphs.

If a Blueprint begins to contain substantial reusable or system-level gameplay logic, consider moving that logic to C++.

---

## Multi-Agent Workflow

The main Claude Code session acts as the **Lead Developer / Producer**.

The Lead is responsible for:

1. understanding the user's requested outcome,
2. deciding whether the task should be handled directly or delegated,
3. decomposing substantial work into concrete tasks,
4. assigning work to the appropriate specialist agents,
5. coordinating dependencies between agents,
6. preventing conflicting changes,
7. integrating the results,
8. requesting QA verification when appropriate,
9. resolving reported failures,
10. giving the user a short final summary.

The Lead owns the overall task and should not delegate blindly.

Use specialist agents when the work naturally belongs to their role.

Do not invoke agents merely to create activity.

For substantial gameplay features, the Lead should normally delegate implementation and verification to appropriate specialist agents.

For very small, local, obvious changes, the Lead may perform the work directly.

### Available Specialist Roles

#### Gameplay Engineer

Owns:

- gameplay C++,
- player mechanics,
- weapons,
- projectiles,
- damage and health,
- enemy gameplay behavior,
- reusable gameplay components,
- mission gameplay logic.

Should avoid changing level layout or presentation unless required for the assigned task.

#### Level Designer

Owns:

- arena geometry,
- primitive mesh composition,
- actor placement,
- level layout,
- simple project-created materials,
- lighting,
- encounter presentation,
- visual readability.

Uses Unreal MCP for editor-side changes when appropriate.

Should avoid modifying gameplay C++ unless explicitly assigned.

#### QA Engineer

Owns verification.

The QA Engineer should:

- build or compile when required,
- run PIE when runtime behavior must be verified,
- test acceptance criteria,
- inspect relevant runtime or build errors,
- report reproducible failures,
- retest fixes.

The QA Engineer should not mark a feature as complete merely because the code appears correct.

The QA Engineer should not silently rewrite major gameplay systems while acting as QA.

#### Architecture Reviewer

Reviews:

- unnecessary coupling,
- duplication,
- inappropriate inheritance,
- premature abstractions,
- poor ownership boundaries,
- systems that will immediately block the next planned mission.

The Architecture Reviewer should recommend changes only when they provide a clear near-term benefit.

Do not perform architecture review for every trivial task.

#### Visual Reviewer

Read-only reviewer of the rendered result.

Reviews:

- visual hierarchy,
- readability of gameplay elements,
- contrast and color roles,
- depth and geometry readability,
- visual restraint and over-design,
- cohesion,
- clarity of gameplay feedback.

The Visual Reviewer judges the visible result, not the implementation intent. A technically correct change that renders badly is still a failure.

For meaningful visual changes, delegate a read-only review to the Visual Reviewer **before** requesting human visual acceptance.

When possible, provide both the previous accepted screenshot and the new screenshot so the reviewer can detect regressions.

Do not allow the Visual Reviewer to implement its own recommendations. It reports; the Lead routes any corrections to the appropriate specialist.

Screenshots used for this review are for the reviewer's own analysis, not for showing to the user — see "Visual Verification and Screenshots" for the project's broader policy.

---


## Model Routing

Use model capability deliberately rather than using the strongest model for every task.

### Lead Developer / Producer

The main Claude session's model is selected by the user for the current phase.

Recommended workflow:

- use **Opus** for design pressure-testing, `/grill-me`, materially ambiguous design decisions, and difficult high-impact architectural reasoning,
- use **Sonnet** for normal implementation, coordination, iteration, and routine integration work.

Do not change the user's selected main-session model automatically unless explicitly requested or clearly necessary.

### Specialist Agents

- **Gameplay Engineer** — Sonnet
- **Level Designer** — Sonnet
- **QA Engineer** — Sonnet
- **Architecture Reviewer** — Opus
- **Visual Reviewer** — Opus

The stronger reviewer model is intentional: review work is occasional and reasoning-heavy, while most production work should remain on Sonnet.

Do not invoke the Architecture Reviewer merely to justify use of Opus.

---

## Design Review and `/grill-me`

Use `/grill-me` as an optional design pressure-test before implementation when a task contains meaningful product or gameplay ambiguity.

Good times to use it include:

- before starting a new mission,
- before a feature that materially changes gameplay rules,
- before decisions that would be expensive to reverse after implementation,
- when the design document leaves multiple materially different interpretations.

Do not use `/grill-me` for trivial, local, or purely mechanical changes.

When using `/grill-me`:

1. read `GAME_DESIGN.md`,
2. focus questions only on ambiguities that materially affect gameplay, implementation, testing, mission flow, or demonstration reliability,
3. do not expand project scope,
4. do not introduce unrelated feature ideas,
5. once decisions are agreed, update `GAME_DESIGN.md` when necessary so the decisions persist outside the conversation,
6. stop the design phase before implementation begins unless the user explicitly asks to continue immediately.

Design review and implementation should remain visibly separable phases.

---

## Agent Coordination Rules

- Avoid multiple agents making overlapping changes to the same files or Unreal assets at the same time.
- Unreal MCP editor-modifying operations must be coordinated by the Lead.
- Do not run overlapping Unreal MCP operations concurrently.
- If two tasks depend on the same asset or source file, perform them sequentially.
- Agents should report changed files and assets back to the Lead.
- Agents should not assume another agent's task has succeeded until the result is available.
- The Lead decides when a failed task should be retried, reassigned, or simplified.
- Prefer one correct implementation over multiple competing alternatives.

---

## Context and Exploration Discipline

- Perform only the task requested by the user.
- Do not perform a broad project inspection without a concrete reason.
- Do not scan all files, assets, actors, Blueprints, or toolsets by default.
- Inspect only the files and assets relevant to the current task.
- Do not enumerate all Unreal MCP tools unless required.
- Do not create long internal plans for simple and unambiguous tasks.
- Do not create multiple alternative implementations of the same feature.
- Do not repeat the same inspection, compilation, or verification without a reason.
- Do not add unrelated cleanup work to the current task.
- Keep final user-facing reports concise.

If the task is unclear and the ambiguity materially affects implementation, ask one short clarification question.

Do not ask for confirmation when the operation is local, reversible, and clearly within the user's request.

---

## Planning

For a simple task:

- execute it directly,
- do not create a separate planning phase,
- do not ask for confirmation if the change is local and reversible.

Provide a short plan before a change that:

- affects several systems,
- changes architecture,
- modifies important existing assets,
- removes or replaces data,
- affects core player control, GameMode, input, or the startup map,
- may be difficult to reverse.

A short plan should cover only:

1. what will change,
2. which parts will be implemented in C++,
3. which parts will be implemented in Blueprint or Unreal assets,
4. how the result will be verified.

Do not produce long architectural essays unless the user asks for them.

---

## Unreal MCP Rules

- Use `unreal-mcp` to inspect or modify Unreal assets when appropriate.
- Never attempt to edit `.uasset` files as ordinary text files.
- Execute Unreal MCP calls sequentially, never concurrently.
- Do not perform overlapping Unreal MCP operations.
- Use the smallest toolset required for the task.
- Search for specific assets or actors rather than listing the entire project when possible.
- Do not perform extra inspection only to make the report more detailed.
- Compile Blueprint assets after modification when applicable.
- Save created and modified assets.
- Save a level only if it was actually changed.
- Do not switch or modify the currently open level without a reason.
- Run PIE when runtime behavior must be verified.
- If an MCP operation fails, inspect the concrete error before retrying.
- Do not repeat the same failing MCP call without changing something relevant.
- Avoid editor operations that conflict with active source compilation.

---

## Unreal Editor Lifecycle

Minimize unnecessary Unreal Editor restarts.

You are allowed to start, stop, and restart the Unreal Editor yourself when needed.

### Starting Unreal Editor

Launch the project with MCP started automatically:

```powershell
UnrealEditor.exe "...\HackGameAI.uproject" -ExecCmds="ModelContextProtocol.StartServer"
```

The default Unreal MCP port is:

```text
8000
```

After launching the editor:

1. Wait for Unreal Editor to finish initializing.
2. Verify that MCP is available on port `8000`.
3. Proceed with editor work only after MCP responds.

Do not repeatedly wait for 90 seconds and retry indefinitely. If MCP does not become available after one normal startup wait and one retry:

- stop retrying,
- report the problem,
- ask the user to manually execute `ModelContextProtocol.StartServer`.

Batch work into editor-on and editor-off phases.

While the editor is running:
- use MCP for assets, maps, Blueprints and runtime inspection,
- use Live Coding for rapid native-code iteration where appropriate,
- test incrementally in PIE,
- save affected assets.

Do not repeatedly close the editor for every C++ edit.

### Restarting or Closing Unreal Editor

Before closing or restarting Unreal Editor:

1. Save all modified assets and levels.
2. Make sure there is no unsaved level state.
3. If appropriate, stop MCP with `ModelContextProtocol.StopServer`.
4. Close the Unreal Editor process normally.

You may restart the editor yourself when a restart is genuinely required.

Before a milestone is presented for QA/human acceptance:

1. save all assets,
2. close the editor,
3. perform a full UnrealBuildTool build,
4. reopen the project,
5. reconnect MCP,
6. verify affected native/Blueprint relationships,
7. rerun the relevant PIE regression checks.

A Live Coding success is an iteration result, not a persistent-build verification.

---

## Source Code and Compilation

- Edit only source files relevant to the task.
- Do not manually modify Unreal Build Tool generated files.
- Do not manually modify generated or cache directories.

Never manually edit:

- `Binaries`
- `DerivedDataCache`
- `Intermediate`
- `Saved`
- `.vs`

- Do not perform a full rebuild when a normal build or module compilation is sufficient.
- After meaningful C++ changes, compile once.
- If compilation fails, fix the concrete failure and compile again.
- Do not fix unrelated warnings without user approval.
- Do not hide errors or warnings by disabling them.
- Do not add plugins or dependencies without explaining why they are needed.
- Do not change Unreal Engine version, compiler version, SDK, or toolchain without user approval.
- Prefer existing engine functionality over adding dependencies.

---

## Asset Naming

Use normal Unreal naming conventions.

Examples:

- `BP_` — Blueprint Actor or Blueprint Object
- `WBP_` — Widget Blueprint
- `BPI_` — Blueprint Interface
- `DA_` — Data Asset
- `DT_` — Data Table
- `M_` — Material
- `MI_` — Material Instance
- `SM_` — Static Mesh
- `SK_` — Skeletal Mesh
- `T_` — Texture
- `NS_` — Niagara System

Do not create unnecessary duplicate assets.

Before creating an asset, verify only its intended name and destination unless more inspection is required.

Use clear names that describe gameplay purpose.

---

## Approved Visual Asset Reuse

Applies to all missions and all visual work, not only the mission currently being built.

**General rule: if a visual solution has already been approved, reuse it by default. Do not reinvent it unless explicitly requested.**

1. **Reuse approved assets from previous missions.** Before creating or assigning a material, texture, effect, or environment visual, check whether an approved version already exists from an earlier mission. If it exists, reuse it rather than authoring a new one.
2. **Approved visuals become the project baseline.** Once a visual element has been manually reviewed and accepted by the user, it is the default standard for every later mission — not just a one-off for the mission it was built in. This covers floor material, exit material and effect, wall materials, lighting/post-process style, and shared VFX (hit feedback, trails, markers).
3. **Do not restore obsolete placeholders.** Never reintroduce an old placeholder, default, legacy, or previously replaced material once a newer approved asset exists for that purpose — including when building a new mission from scratch.
4. **Prefer reuse over recreation.** Do not create duplicate materials or near-identical copies of an existing approved asset unless there is a clear gameplay or art-direction reason. Share the same asset across missions whenever practical rather than branching a per-mission variant.
5. **Maintain visual consistency across missions.** Later missions should read as part of the same game, not independently built prototypes. Differences between missions should come from gameplay, layout, enemies, or an explicit user request — never from accidental drift in shared materials.
6. **Check the previous mission before making a visual decision.** When unsure how a recurring element should look, inspect the latest approved implementation (`Docs/ArtDirection/README.md` holds the canonical baseline and its measured values) and match it rather than guessing or re-deriving from scratch.

Before creating a new material/effect for a new mission, check `Docs/ArtDirection/README.md` and the prior mission's assets first. If an equivalent approved asset exists, apply it directly (reparent/reuse) rather than authoring a new one.

---

## Change Safety

- Do not delete existing assets or code without explicit approval unless deletion is clearly part of the user's task.
- Do not substantially restructure working systems without a clear reason.
- Do not perform mass renames or mass asset moves.
- Preserve working functionality while extending the project.
- Prefer small reversible changes.
- Inform the user before a difficult-to-reverse operation.

### Git Safety

Do not use destructive Git operations.

Never run:

- `git reset --hard`
- `git clean -fd`
- forced checkout operations that discard local work
- force push

Do not commit, push, create branches, merge branches, or rewrite Git history unless the user explicitly requests it.

Do not discard user changes that were not created by the current task.

Git is the user's recovery mechanism and should be treated conservatively.

---

## Testing and Acceptance

Choose verification proportional to the task.

### Asset Changes

When modifying Unreal assets:

- verify the asset exists,
- compile Blueprint assets when applicable,
- inspect only the most important resulting properties,
- save the asset.

### C++ Changes

When modifying C++:

- compile the project or relevant module,
- verify the concrete build result,
- fix task-related failures.

### Gameplay Features

Gameplay features should be tested in PIE when runtime behavior is required to confirm correctness.

For substantial gameplay tasks:

1. define or infer a small set of acceptance criteria,
2. build or compile,
3. run PIE,
4. verify the required behavior,
5. report PASS or concrete failures,
6. fix failures,
7. retest the affected behavior.

Do not treat successful compilation alone as proof that gameplay works.

Do not perform long autonomous exploratory testing unless requested.

---

## Motion-Dependent Visual Verification

Do not autonomously tune motion-dependent gameplay or visual effects unless the
available tools can reliably observe the effect in motion.

Examples include:
- movement trails and particles
- animations
- recoil
- camera shake
- hit reactions
- enemy movement
- projectile readability
- timing-dependent VFX

A static screenshot is NOT valid evidence for tuning a temporal effect.

If MCP/tooling cannot reliably:
1. trigger the required continuous gameplay behavior, and
2. observe the resulting effect over time,

then:

- do not infer that an absent effect is too weak,
- do not repeatedly increase intensity, size, lifetime, opacity, speed, or similar parameters,
- do not substitute teleportation or isolated frames when they do not reproduce the real behavior,
- stop autonomous tuning,
- report the tooling limitation,
- request human in-motion verification.

Distinguish between:
- implementation/wiring failure,
- effect present but incorrectly tuned,
- inability to observe the effect reliably.

When observation is unreliable, human visual acceptance is authoritative.

---

## Visual Verification and Screenshots

Do not take screenshots, capture the editor viewport, or show screenshots to the user as part of normal verification.

Do not spend time building screenshot workflows or trying to visually prove a result to the user.

Use MCP/editor state, asset properties, Blueprint compilation results, logs, and other structured editor information for technical verification instead.

When visual judgement is required, finish the implementation and tell the user what is ready for manual visual inspection. The user has their own tools and will evaluate the result directly in Unreal Editor.

Exception: the Visual Reviewer agent may still capture and use screenshots internally as part of its own defined review process (see "Visual Reviewer" above) — that is how it forms its judgement. Even then, it should report its findings as text, not relay raw screenshots back to the user.

---


## Human Acceptance Gate

AI verification does not replace final human acceptance for a mission-level milestone.

For completion of a mission, use this sequence:

1. implementation is completed,
2. QA verifies explicit acceptance criteria,
3. QA reports `PASS`,
4. the Lead reports the verified result to the user,
5. the user performs a short manual smoke test,
6. only after the user's acceptance should the milestone be treated as ready for commit, merge, or tag.

A QA `PASS` means the automated/agent verification succeeded. It does **not** mean the Lead may commit, merge, tag, or otherwise finalize the milestone without explicit user instruction.

If the user's manual smoke test finds a problem:

- treat the mission as not accepted,
- identify the smallest responsible area,
- delegate the fix,
- run focused QA again,
- return to the human acceptance gate.

This project is agentic but human-supervised.

---

## Mission-Level Acceptance

A mission is not complete until its required gameplay loop has been verified.

Typical Mission 1 checks may include:

- player can move,
- player can aim,
- player can shoot,
- projectile behaves correctly,
- walls block or constrain movement as intended,
- target or objective can be completed,
- mission completion triggers correctly.

Typical Mission 2 checks may include:

- all Mission 1 behavior still works,
- stationary enemies fire,
- enemy projectiles can damage the player,
- player projectiles can damage enemies,
- enemies can be destroyed,
- mission completion triggers after required enemies are defeated.

Typical Mission 3 checks may include:

- all earlier gameplay remains functional,
- enemies move according to intended behavior,
- moving enemies still attack correctly,
- encounter can be completed,
- no obvious regression blocks gameplay.

Acceptance criteria should remain small and directly related to the requested feature.

---

## Scope Control

The project is intentionally small.

Do not add systems merely because a larger game might eventually need them.

Avoid unnecessary implementation of:

- inventories,
- save systems,
- online features,
- complex AI frameworks,
- large ability systems,
- procedural generation frameworks,
- generalized plugin architectures,
- elaborate data pipelines,
- advanced animation systems,
- external asset workflows,

unless the user explicitly requests them or they become clearly necessary.

When a simple implementation is sufficient for the current three-mission game, prefer the simple implementation.

---

## Final Report

After completing a task, report only:

### Completed

One or two sentences describing the result.

### Changes

List:

- changed source files,
- created or modified Unreal assets.

### Verification

Report:

- build or compilation result,
- Blueprint compilation result when relevant,
- PIE result when relevant,
- QA `PASS` / `FAIL` when a substantial gameplay feature was verified,
- whether a mission-level milestone is awaiting human acceptance,
- any known issue that requires a user decision.

Do not describe every tool call or internal reasoning step.

Keep the final report short and practical.
