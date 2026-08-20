# HackGameAI — Game Design

> Decisions from the pre-production design review are incorporated throughout.
> Sections 17 and 18 record the agreed technical decisions and the Mission 1 acceptance criteria.
> Values marked as *initial tuning values* may be adjusted during implementation without a new design decision.

## 1. Purpose

HackGameAI is a very small top-down action game created as a focused demonstration of AI-assisted and multi-agent game development in Unreal Engine.

The project is intentionally limited in scope. The goal is not to build a content-heavy or visually realistic game, but to create a complete, understandable gameplay experience that AI agents can implement, test, and iterate on with minimal manual intervention.

The game takes inspiration from abstract hacking sequences such as those found in NieR: Automata, but it should not reproduce copyrighted assets, layouts, UI, audio, or other distinctive content.

---

## 2. Design Pillars

### Simple

The game should be immediately understandable.

The player moves, aims, shoots, avoids danger, and completes short arenas.

### Readable

Every gameplay element should be visually obvious from shape, movement, and color/material treatment.

The game should remain readable even though it uses only simple geometry.

### Progressive

Each mission introduces exactly one major new layer of complexity:

1. navigation, and a stationary destructible target,
2. stationary hostile enemies that shoot back,
3. moving hostile enemies.

The hostile actor progression is deliberately incremental:

| Mission | Actor | Moves | Shoots | Destructible |
| --- | --- | --- | --- | --- |
| 1 | Destructible target | No | No | Yes |
| 2 | Stationary enemy | No | Yes | Yes |
| 3 | Moving enemy | Yes | Yes | Yes |

### Small

Every system should justify its existence within the three-mission game.

No feature should be added merely because a larger game might eventually need it.

---

## 3. Perspective and Presentation

### Camera

The game uses a **follow camera with fixed height, angle and distance**.

The camera tracks the player's position only. It does not rotate, zoom, lag, or offset toward the aim direction, and it does not perform collision testing against level geometry.

Camera setup (initial tuning values):

- spring arm length approximately `1400`,
- pitch approximately `-60°`,
- field of view `90`,
- camera lag disabled,
- spring arm collision testing disabled,
- spring arm rotation inheritance disabled for pitch, yaw and roll.

Rotation inheritance must be disabled because the player pawn yaws to face the mouse. Without this, the camera would rotate with the aim direction.

The arena exists primarily on a single gameplay plane.

### Materials and Lighting

All gameplay materials use the **Unlit** shading model with emissive-driven color.

The project uses:

- one parameterized unlit master material,
- material instances for each gameplay role,
- one shared PostProcessVolume providing restrained bloom.

No directional light, sky light, or per-map lighting rig is required.

Bloom must remain restrained enough that projectiles and silhouettes stay sharply readable.

Because there is no lighting, depth and separation are communicated through **value and color contrast** between floor, walls and gameplay actors rather than through shading or shadows.

Initial palette (tuning values, not hard requirements):

| Role | Color |
| --- | --- |
| Floor | dark near-black |
| Walls | desaturated grey |
| Player | cyan |
| Player projectiles | cyan / white |
| Mission exit | green |
| Enemies | red / orange |
| Enemy projectiles | magenta |

### General presentation

The visual presentation should use:

- simple geometric primitive meshes,
- strong visual contrast,
- minimal visual clutter,
- clear projectile visibility.

The visual style should look deliberate and abstract rather than like an unfinished realistic game.

No external art assets are required.

### Frozen Visual Direction

**This art direction is APPROVED and FROZEN. Do not explore alternative art directions unless the user explicitly requests it.**

The canonical visual reference is stored at `Docs/ArtDirection/` — see that folder's `README.md` for the concept image and the full rules.

Approved visual vocabulary:

| Element | Target |
| --- | --- |
| Floor | dark matte graphite, subtle technological grid/panel pattern |
| Wall tops | bright, clean, near-white |
| Wall sides | darker cool-grey |
| Overall | strong contrast between dark floor and bright geometry |
| Player | cyan glow, plus a cyan comet-like movement trail |
| Hostile / blocked state | orange-red |
| Objective | bright cyan-green |
| Post-processing | restrained bloom, minimal abstract sci-fi |

The concept reference is a **STYLE reference only**. The following are explicitly NOT adopted from it:

- its arena shape and layout,
- its camera,
- its player silhouette,
- any gameplay elements visible in it (rotating shooter enemies, striped locked gate, obstacle block clusters). These are later-mission or out-of-scope content and must not be built from the reference image.

Locked and not to be changed for visual reasons: camera, map layout, gameplay scale, player silhouette, gameplay logic.

### Approved rendering baseline (measured in PIE)

These measured values are the accepted state. Treat them as the reference point for any future visual change.

| Surface | Measured (0-255) |
| --- | --- |
| Floor panel | 12 |
| Floor grid seam | ~8 |
| Wall side | 74 |
| Wall top | 212 |
| Wall top : side ratio | 2.90 : 1 |
| Target | (237, 91, 44) |
| Player | (134, 237, 238) |
| Exit (active) | (114, 227, 167) |
| Cyan player pool on floor, peak | (1, 149, 172) |

Post-process on `L_Mission01`: exposure **locked** (auto-exposure min = max = 1.0, bias 0, physical camera exposure off), bloom 0.65, motion blur 0, vignette 0.

**The exposure lock is load-bearing.** Unclamped auto-exposure re-normalises this deliberately dark scene to mid-grey, which silently invalidates every authored material value. Do not re-enable auto-exposure.

`M_HackFloor` uses the **Default Lit** shading model with an emissive term. Its dark base comes from emissive; its albedo responds to lights. This is intentional — it is what allows the player's cyan light to pool on the floor. Any light added to a mission will also light the floor, so add lights deliberately.

### Visual change process

For any future visual task:

1. capture the current PIE state via MCP first, and treat that render as the structural baseline,
2. modify only the requested element,
3. compare the result against the concept reference,
4. delegate a read-only review to the Visual Reviewer,
5. stop for human review.

Judge the rendered frame, never the parameter values. Numeric ordering is not perceptual ordering — thin emissive lines through a bloom pass read far brighter than a flat surface at the same numeric value.

---

## 4. Player

The player is represented by a simple geometric object with a **directional silhouette** so that facing is readable from shape alone (for example a cone, or a body with a small nose block).

The exact primitive may be chosen during implementation based on readability.

The player pawn derives from `ACharacter` and uses `CharacterMovementComponent` for wall collision and sliding.

### Controls

- `WASD` — move on the gameplay plane, in world space.
- Mouse — aim toward the cursor.
- Left Mouse Button — fire; **hold to fire continuously**.
- `Esc` — pause (disabled on the main menu).

Input uses **Enhanced Input**: `IMC_Player`, `IA_Move`, `IA_Fire`, `IA_Pause`.

There is no `IA_Look` asset. Aiming reads the mouse position directly.

The controls should feel immediate and responsive.

The player should not require animation.

Movement tuning (initial values):

- `MaxWalkSpeed` approximately `600`,
- `MaxAcceleration` approximately `4000`,
- `BrakingDecelerationWalking` approximately `4000`,
- `GroundFriction` approximately `8`.

High acceleration and braking are preferred over instant velocity changes, because instant velocity fights the capsule at corners and degrades wall sliding.

### Aiming

The player aims toward the mouse position projected onto the gameplay plane.

Aim is computed by deprojecting the mouse position to a world ray and intersecting it with a horizontal plane at the pawn's height. Aiming never traces world geometry, so it remains stable when the cursor is over a wall or outside the arena.

The PlayerController sets the control rotation from this aim direction; the pawn follows via `bUseControllerRotationYaw`.

The mouse cursor is **visible at all times**, in both menus and gameplay, using the engine `EMouseCursor::Crosshairs` cursor.

Mouse capture is disabled and the cursor is not locked to the viewport. The same configuration applies on every map, so there is no mode switching at mission or menu boundaries.

A custom software cursor is not implemented. It may be considered later as optional polish.

### Shooting

The player fires a visible projectile in the current aim direction.

Firing is hold-to-fire with a fixed cooldown of approximately `0.15 s` (initial tuning value).

Projectiles should:

- travel at a readable speed,
- have a limited lifetime or be destroyed when appropriate,
- interact consistently with walls and enemies,
- be easy to distinguish from hostile projectiles.

Shooting is available in **all three missions**, including Mission 1. Mission 1 contains nothing to shoot and does not require shooting for completion.

---

## 5. Health and Failure

The player has `5 HP`.

Health is present from **Mission 1 onward**, so the player actor remains structurally identical across all three missions. Mission 1 contains no damage source, so damage and death behavior are not part of Mission 1 acceptance; Mission 1 verifies only that the HUD correctly reads the health component. Damage and death become testable in Mission 2.

Health rules:

- no regeneration,
- no invulnerability window,
- each projectile deals 1 damage and is destroyed on impact, so a single projectile can damage at most once,
- enemy health is tuned separately from player health.

Health is stored in a shared `UHealthComponent` used by both the player and enemies.

When the player loses all health:

- gameplay stops,
- failure is clearly communicated by the mission-failed overlay,
- the player can retry the current mission.

A sophisticated death screen or checkpoint system is not required.

---

## 6. Enemies

Enemies are represented by simple geometric actors clearly distinguishable from the player.

All enemies should be understandable primarily through behavior rather than art.

Enemies use the same `UHealthComponent`, weapon cadence and projectile system as each other.

### Destructible Target

Introduced in Mission 1.

Behavior:

- remains in a fixed position,
- **does not move**,
- **does not shoot**,
- has no perception, targeting, or AI of any kind,
- takes damage from player projectiles,
- is destroyed when health reaches zero.

The target is Mission 1's introduction to the damage and destruction systems. It is deliberately inert, so Mission 1 still contains nothing that can damage the player.

It reuses the shared `UHealthComponent` (Team = Enemy) and the existing projectile damage path — no new damage architecture.

Initial tuning value: target health `3 HP`.

#### Hit feedback

Every successful hit must be immediately obvious. Because the game has no lighting and no health bars, feedback is carried entirely by emissive value and shape:

- a brief emissive brightness flash on every successful hit,
- a subtle scale pulse or squash on every successful hit,
- optionally a small, cheap, project-created impact burst,
- a stronger version of the same feedback on the killing hit, played out before the target disappears.

No health bar. No external assets. The feedback lives on the target actor itself — it must not become a generalized hit-reaction framework.

On the killing hit the target's collision is removed immediately, so the path opens the instant the target dies, while the death feedback plays out before the actor is destroyed.

### Stationary Enemy

Introduced in Mission 2.

Behavior:

- remains in a fixed position,
- is **always active** — no perception, no line-of-sight check, no range check,
- aims at the player,
- fires hostile projectiles on a regular cadence,
- can take damage,
- is destroyed when health reaches zero.

There is **no predictive pre-fire telegraph**. Dodging relies on projectile readability, regular cadence and travel time rather than a warning animation.

A brief muzzle flash or effect at the exact moment of firing is acceptable as feedback, but it must not function as advance warning.

Projectiles blocked by walls simply stop at the wall, which reads correctly without any line-of-sight logic.

Initial tuning values:

- fire interval approximately `1.3 – 1.6 s`,
- enemy projectile speed approximately `1000 – 1200`,
- enemy health `3 HP`.

The enemy should not require complex perception or AI frameworks.

### Moving Enemy

Introduced in Mission 3.

Movement uses **simple direct steering in C++**. The project does not use NavMesh, `AIController` movement, or Behavior Trees.

Behavior — approximately two movement states:

- `Approach` — move toward the player while outside preferred range,
- `Strafe` — strafe around the player while within preferred range.

Moving enemies fire using the same combat system as stationary enemies.

`CharacterMovementComponent` handles wall sliding. Because there is no pathfinding, **Mission 3's arena must be intentionally open** — perimeter walls plus at most one or two cover blocks, with no corridors or concave pockets that could trap a steering enemy.

---

## 7. Projectiles

Player and enemy projectiles must be visually distinct.

Projectiles should provide clear gameplay information:

- who fired them,
- where they are moving,
- whether the player should avoid them.

All projectiles travel in straight lines using `ProjectileMovementComponent`, with no gravity and no bouncing. Complex ballistic simulation is not required.

### Collision and friendly fire

No custom collision channels are added to the project. Projectile collision responses are:

- **block** `WorldStatic` and `WorldDynamic` — projectiles stop on walls and are destroyed,
- **overlap** `Pawn`,
- ignore the actor that fired them.

On pawn overlap, the projectile compares the `Team` value stored on the target's `UHealthComponent`:

- different team → apply damage and destroy the projectile,
- same team, or no health component → ignore and continue travelling.

This makes friendly fire structurally impossible without a custom collision channel matrix, and prevents enemies from blocking or damaging each other in Missions 2 and 3.

Initial speed values:

- player projectile approximately `2500`,
- enemy projectile approximately `1000 – 1200`,
- player movement approximately `600`.

The speed gap between enemy projectiles and player movement is the primary difficulty dial.

---

## 8. Arena Rules

Each mission takes place in a compact enclosed arena of roughly `4000 x 4000 uu` (initial layout value).

Walls are built from scaled engine `Cube` static meshes, approximately `300` tall and `100` thick (initial values).

Exact arena dimensions and wall height/thickness may be adjusted during implementation if readability or camera framing requires it.

Walls:

- define the playable area,
- block player movement,
- block projectiles,
- have clear collision.

Arena layouts should remain small enough that the whole encounter is easy to understand.

Avoid complex verticality.

---

# 9. Mission Structure

### Required Enemies

Where a mission's objective says "destroy all required enemies," **required** means the specific enemies designated per-mission as gating completion — not necessarily every hostile actor placed in the level.

A mission may contain non-required hostile or destructible actors (obstacles, cover-breakers, optional targets) that do not gate completion. Which enemies are required, and what completing them unlocks (the exit directly, or something gating the exit such as a barrier), is a per-mission design decision and should be stated explicitly in that mission's section.

## Mission 1 — Navigation

### Purpose

Teach the player how movement and the camera work with the smallest possible gameplay setup.

### Content

- player,
- follow camera,
- enclosed arena,
- walls forming a simple route,
- one stationary destructible target,
- mission exit,
- no hostile enemies and no damage source of any kind.

### Layout

A simple **L- or U-shaped route**.

The exit is not visible from the player start, so the player must traverse roughly two camera screens and experience the follow camera doing its job.

The layout must remain trivial and completable well under a minute. No maze, no wrong turns, no puzzle.

### Objective

Destroy the stationary target, then reach the mission exit.

### Target

One `BP_DestructibleTarget` placed along the route, positioned so the player encounters it before reaching the exit.

It does not move, does not shoot, and cannot harm the player. It exists to introduce shooting and destruction.

### Exit

`BP_MissionExit` — a box trigger plus an emissive pad mesh.

The exit is **inactive until the target is destroyed**. While inactive it must read as inactive (for example a dimmed or differently-coloured material) and overlapping it does nothing.

Once the target is destroyed the exit becomes active. Completion then fires **immediately on player overlap** and is guarded against re-triggering.

There is no dwell requirement.

### New Concept

Navigation, plus shooting a destructible target.

### Completion

On overlap with the **active** exit, the exit calls `CompleteMission()` on the mission GameMode.

### Design Constraint

Mission 1 should remain intentionally trivial.

The target does not move, does not shoot, and poses no threat. Do not add hostile enemies, combat hazards, moving obstacles, puzzles, or other systems simply to make the mission feel larger.

Exactly one target is required. Do not add multiple targets or a target-count objective — that is Mission 2's shape, not Mission 1's.

---

## Mission 2 — Stationary Threats

### Purpose

Introduce combat and projectile avoidance without adding enemy movement.

### Content

- everything required from Mission 1,
- stationary hostile enemies (shooters),
- non-required destructible obstacles (reusing Mission 1's destructible target),
- hostile projectiles,
- player health,
- enemy health,
- a locked barrier gating the exit,
- combat arena layout with a separate exit chamber.

### Objective

Destroy the required enemies (the stationary shooters) to open the barrier, then reach the exit.

### New Concept

Stationary ranged threats, and a barrier-gated objective — completion is no longer "reach the exit directly" (Mission 1) but "clear the required enemies, which opens the path to the exit."

### Completion

The exit lives in a separate small chamber connected to the main arena, blocked by a locked barrier (`AMissionBarrier`) until its required enemies are destroyed.

`AMissionBarrier` holds an `EditInstanceOnly` list of required enemies, set per level instance. When all of them report death, the barrier disables its collision and hides — a purely physical unlock. It carries no completion logic itself; reaching the exit still calls `CompleteMission()` exactly as in Mission 1, and the exit's own `RequiredTarget` is left unset (the barrier is what gates access, not exit-side logic).

The **required enemies are the stationary shooters only.** The destructible obstacles (reused from Mission 1) are present as combat-arena cover/obstacles but are explicitly **not required** — destroying them does not affect the barrier.

### Failure

The mission fails if player health reaches zero.

The player can retry the mission.

### Encounter Direction

Initial encounter: **4 stationary shooters at 3 HP each**, placed near the arena corners for symmetric coverage, plus **6 non-required destructible obstacles** placed as a readable line in front of the barrier — they add cover/obstruction but are not part of the win condition.

Enemies should be placed so that hostile projectile patterns create movement pressure without overwhelming the player. The player starts behind simple cover (a U-shaped wall arrangement, open toward the encounter) rather than in the open.

The encounter should be short.

---

## Mission 3 — Moving Threats

### Purpose

Demonstrate the next step in gameplay complexity by adding enemy movement.

### Content

- everything required from Mission 2,
- moving hostile enemies,
- an intentionally open arena that gives the player room to reposition and does not require pathfinding.

### Objective

Destroy all required enemies.

### New Concept

Moving hostile threats.

### Completion

The mission ends when all required enemies are destroyed.

### Failure

The mission fails if player health reaches zero.

The player can retry the mission.

### Encounter Direction

Initial encounter: **2 stationary enemies and 2 moving enemies**.

Mixing both enemy types demonstrates reuse of Mission 2 systems rather than replacing them.

Movement should noticeably change how the player approaches combat compared with Mission 2.

---

## 10. Mission Flow and Mission Selection

### Maps

The game uses four maps:

- `L_MainMenu`
- `L_Mission01`
- `L_Mission02`
- `L_Mission03`

Mission selection uses `OpenLevel`. Retry reloads the current map. This gives full state isolation between attempts and between missions, and removes the need for any teardown logic.

### Main menu

`L_MainMenu` is a minimal map containing a title and four buttons:

- `Mission 1`
- `Mission 2`
- `Mission 3`
- `Quit`

All three missions are available from the start. No unlock system, campaign progression, save data, or persistent mission state is required.

**All mission buttons must always point at valid maps.** Missions that are not yet implemented ship as **playable shells** — floor, perimeter walls, `PlayerStart`, correct GameMode and player pawn, working camera, input and shooting, and a working `Esc` pause with Return to Menu — rather than as broken buttons or "not implemented" placeholder screens.

`Quit` calls `QuitGame`, which ends the PIE session when run in-editor.

### Mission completion flow

The mission-complete overlay shows:

- `Next Mission`
- `Return to Menu`

Flow:

- Mission 1 → `Next Mission` opens Mission 2,
- Mission 2 → `Next Mission` opens Mission 3,
- Mission 3 shows `GAME COMPLETE` and only `Return to Menu`.

### Failure flow (Mission 2 onward)

The mission-failed overlay shows:

- `Retry` — reloads the current map,
- `Return to Menu`.

### Pause

`Esc` opens a minimal pause menu with `Resume` and `Return to Menu`. Pause is disabled on the main menu.

### Transition rules

The game is paused underneath completion, failure and pause overlays.

There are **no automatic timed transitions**. The player always explicitly chooses what happens next.

---

## 11. UI

UI should remain minimal.

Widgets:

| Widget | Contents | First needed |
| --- | --- | --- |
| `WBP_MainMenu` | Title, Mission 1, Mission 2, Mission 3, Quit | Mission 1 |
| `WBP_MissionHUD` | Player health only, drawn as 5 discrete segments | Mission 1 |
| `WBP_PauseMenu` | Resume, Return to Menu | Mission 1 |
| `WBP_MissionComplete` | Next Mission, Return to Menu (or `GAME COMPLETE` + Return to Menu on Mission 3) | Mission 1 |
| `WBP_MissionFailed` | Retry, Return to Menu | Mission 2 |

The HUD is shown from Mission 1 onward, even though nothing can damage the player in Mission 1.

No mission-name label is required.

Avoid creating elaborate HUD systems.

If the gameplay can communicate information directly through the world, prefer that over additional UI.

---

## 12. Audio

Audio is optional for the initial implementation.

The game must remain understandable and playable without custom audio assets.

If simple engine-available or project-created audio becomes useful later, it may be added only if it does not distract from completing the core game.

---

## 13. Difficulty and Duration

HackGameAI is a demonstration, not a challenge-focused game.

Target experience:

- immediately understandable,
- easy to complete,
- short enough to demonstrate quickly,
- increasingly active from mission to mission.

A full playthrough should ideally take only a few minutes.

Difficulty tuning should favor reliable demonstration over player punishment.

---

## 14. Out of Scope

The following are explicitly not required:

- character animation,
- skeletal meshes,
- realistic art,
- external asset packs,
- story or dialogue,
- inventory,
- progression systems,
- upgrades,
- save games,
- checkpoints beyond retrying the current mission,
- multiplayer,
- online features,
- procedural level generation,
- advanced navigation AI,
- NavMesh and `AIController` movement,
- Behavior Trees,
- Gameplay Ability System,
- complex physics,
- custom collision channels,
- elaborate menus beyond the simple mission-selection screen,
- large levels,
- boss encounters,
- additional missions.

Do not add these without an explicit design decision.

---

## 15. Definition of Done

HackGameAI is complete when:

1. a main menu allows direct selection of Mission 1, Mission 2, or Mission 3,
2. all three missions can be launched without editor intervention,
3. Mission 1 demonstrates navigation,
4. Mission 2 adds working stationary ranged enemies,
5. Mission 3 adds working moving enemies,
6. player shooting and enemy damage work reliably,
7. player damage and failure work reliably,
8. mission completion works reliably,
9. the game communicates success and failure clearly,
10. the game can return to a predictable post-mission state, preferably the mission selection menu,
11. the project can be demonstrated without manual editor intervention during normal play,
12. the complete game has been tested across all three missions.

---

## 16. Design Decision Policy

When a detail is not defined in this document:

1. prefer the simplest solution,
2. prefer the solution that is easiest to test,
3. prefer consistency with existing implemented systems,
4. avoid increasing scope,
5. ask the user only when the choice materially changes the gameplay or project direction.

New design decisions that materially change gameplay should be reflected in this document.

---

## 17. Agreed Technical Decisions

These decisions were resolved during the pre-production design review. They are recorded here because they are expensive or awkward to change once implementation begins.

### Content structure

| Path | Contents |
| --- | --- |
| `/Game/HackGameAI/Maps` | `L_MainMenu`, `L_Mission01`, `L_Mission02`, `L_Mission03` |
| `/Game/HackGameAI/Core` | Mission GameMode Blueprints, PlayerController Blueprint if needed |
| `/Game/HackGameAI/Player` | `BP_HackPlayerCharacter` |
| `/Game/HackGameAI/Missions` | `BP_MissionExit` and mission-specific actors |
| `/Game/HackGameAI/UI` | Widget Blueprints |
| `/Game/HackGameAI/Materials` | Unlit master material and material instances |
| `/Game/HackGameAI/Enemies` | Enemy Blueprints (Mission 2 onward) |
| `/Game/HackGameAI/Input` | `IMC_Player`, `IA_Move`, `IA_Fire`, `IA_Pause` |

Folders are created only when first needed.

### C++ surface

C++ owns gameplay rules and reusable systems; Blueprint owns composition, meshes, materials and presentation.

| Class | Base | Responsibility | First needed |
| --- | --- | --- | --- |
| `AHackPlayerCharacter` | `ACharacter` | Movement, camera rig, firing | Mission 1 |
| `AHackPlayerController` | `APlayerController` | Mouse aiming, cursor config, pause input | Mission 1 |
| `AMissionGameMode` | `AGameModeBase` | `CompleteMission()`, `FailMission()`, `NextMissionMap`, `bIsFinalMission` | Mission 1 |
| `UHealthComponent` | `UActorComponent` | HP, `Team` enum, death event | Mission 1 |
| `AHackProjectile` | `AActor` | Straight-line travel, team-filtered damage | Mission 1 |
| `AMissionExit` | `AActor` | Overlap trigger that completes the mission; activates only once its required target is destroyed | Mission 1 |
| `ADestructibleTarget` | `AActor` | Inert stationary target with a `UHealthComponent`; broadcasts its destruction | Mission 1 |
| Enemy classes | `ACharacter` | Stationary and moving enemies | Mission 2 / 3 |

Each mission map uses a small GameMode Blueprint subclass (for example `BP_Mission01GameMode`) set as the GameMode Override in World Settings, carrying that mission's `NextMissionMap` and `bIsFinalMission` values.

`AMissionGameMode` deliberately has no objective-type enum and no virtual objective hooks. Mission 1's exit and Mission 2's enemy counter both call the same `CompleteMission()`.

### Damage

Damage is routed through the engine damage path: projectiles call `UGameplayStatics::ApplyDamage`, and `UHealthComponent` binds to `OnTakeAnyDamage`. Projectiles never cast to specific actor classes.

### Deferred refactor

Firing logic lives on the player character in Mission 1. If the Mission 2 enemy needs identical firing behavior, it should be extracted into a shared `UWeaponComponent` at that point. It is not extracted preemptively.

### Project configuration changes

- `GameDefaultMap` and `EditorStartupMap` must be repointed from `/Engine/Maps/Templates/OpenWorld` to `/Game/HackGameAI/Maps/L_MainMenu`.
- `DefaultViewportMouseCaptureMode` must change from `CapturePermanently_IncludingInitialMouseDown` to `NoCapture`.
- `DefaultViewportMouseLockMode` must change from `LockOnCapture` to `DoNotLock`.
- No collision channels or collision profiles are added.

### Known implementation gotchas

- The spring arm must have rotation inheritance and collision testing disabled, or the camera will spin with the aim direction and push in near walls.
- Input mode is `GameAndUI` with the cursor always shown, on every map.
- Because completion, failure and pause overlays pause the game, the PlayerController must still process the pause input while paused (`bShouldPerformFullTickWhenPaused`), or pause becomes unexitable by keyboard.

### Verification target

PIE is the verification target for all agent QA. A packaged build is not required for normal acceptance.

"Demonstrable without manual editor intervention" means: start PIE on `L_MainMenu`, and everything from that point is reachable in-game.

---

## 18. Mission 1 Acceptance Criteria

Mission 1 is not complete until every criterion below has been verified in PIE.

### Build

1. The project compiles with no errors.
2. All created Blueprints and widgets compile with no errors.

### Menu and flow

3. PIE starts on `L_MainMenu` with the cursor visible and all four buttons present.
4. `Mission 1` opens `L_Mission01` and spawns the player.
5. `Mission 2` and `Mission 3` open their playable shell maps and spawn the player — no dead buttons, no empty void.
6. `Esc` during a mission opens the pause menu; `Resume` returns to play; `Return to Menu` returns to `L_MainMenu`.
7. Pause is not available on `L_MainMenu`.

### Player control

8. `WASD` moves the player in world space at a responsive speed.
9. The player pawn rotates to face the mouse cursor.
10. Aim remains correct and stable when the cursor is over a wall and when it is outside the arena bounds.
11. The camera follows the player at a constant height, angle and distance, and does not rotate when the player aims.
12. The camera does not push in or change distance when the player moves close to a wall.

### Shooting

13. Holding the left mouse button fires projectiles continuously at the cooldown rate.
14. Projectiles travel in a straight line along the aim direction.
15. Projectiles are destroyed on contact with walls.
16. Projectiles do not damage or block the player who fired them.
17. Projectiles expire after their lifetime if they hit nothing.

### Arena

18. Walls block player movement; the player cannot leave the arena.
19. The player can traverse the L/U route from the player start to the exit.

### HUD

20. The health HUD is visible and displays 5 of 5 segments for the whole mission.

### Destructible target

21. Exactly one destructible target exists in Mission 1.
22. The target never moves and never fires.
23. Player projectiles damage the target.
24. The target is destroyed after taking its configured damage.
25. The target cannot damage the player, and player health stays at 5 of 5 throughout.
25a. Every successful hit produces an immediate, visible reaction on the target (emissive flash and scale pulse).
25b. The killing hit produces a stronger reaction that is visible before the target disappears.
25c. Hit feedback never changes damage values, target health, or the number of hits required.

### Completion gating

26. Before the target is destroyed, the exit is visibly inactive and overlapping it does **not** complete the mission.
27. After the target is destroyed, the exit becomes visibly active.
28. Entering the active exit immediately shows the mission-complete overlay.
29. `Next Mission` opens `L_Mission02`.
30. `Return to Menu` opens `L_MainMenu`.
31. The completion overlay cannot be triggered twice by re-entering the exit.

### Frozen state while an overlay is active

Applies to both the mission-complete overlay and the pause menu.

32. The camera/view remains completely stable — no rotation, drift, or spin, regardless of mouse movement.
33. The player pawn does not move.
34. The player pawn does not rotate or continue aiming.
35. Firing does not occur.
36. UI interaction (buttons) still works while gameplay is frozen.

### Scope

37. No hostile enemies, hostile projectiles, or damage sources exist in Mission 1 — the destructible target is inert and is not a hostile enemy.
38. No Mission 2 or Mission 3 gameplay exists in Mission 1.
