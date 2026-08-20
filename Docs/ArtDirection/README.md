# HackGameAI — Canonical Visual Reference

**Status: APPROVED AND FROZEN.**

Do not explore alternative art directions unless the user explicitly requests it.

This folder is the single source of truth for the game's visual direction. `GAME_DESIGN.md` section 3 ("Frozen Visual Direction") points here.

---

## The reference image

`concept_arena_reference.png`

This image is the approved visual target for the game.

It is a **STYLE reference only**. It is not a layout reference, not a camera reference, and not a feature list.

---

## Adopt from the reference

- dark matte graphite floor,
- subtle technological grid/panel pattern on the floor,
- bright clean wall tops,
- darker cool-grey wall sides,
- strong contrast between the dark floor and bright geometry,
- cyan player glow,
- cyan comet-like movement trail,
- orange/red language for hostile or blocked states,
- bright cyan/green language for objectives,
- restrained bloom,
- minimal abstract sci-fi presentation.

## Do NOT adopt from the reference

The reference image contains content that must **not** be built from it:

- **its arena shape and layout** — the real Mission 1 layout is an L-bend leading to a dead-end corridor, and it is accepted. Do not reshape the arena to match the image.
- **its camera** — the real camera is a follow camera at fixed height/angle/distance, ~1400 arm length, -60 degrees pitch, FOV 90.
- **its player silhouette** — the real player silhouette is accepted as-is.
- **rotating shooter enemies** — Mission 2 content. Mission 1 has exactly one inert destructible target.
- **the obstacle block clusters** — not part of the accepted layout.

The striped locked gate was originally listed here as unbuilt. It has since been implemented as a real mechanic — Mission 2's exit chamber barrier, reused in Mission 3. See `M_LockedGate` / `M_LockedGateGlow`, `BP_MissionBarrier`, `BP_MissionBarrier_Side` and `BP_GateBeam`. Take the gate's *appearance* from those assets, not from the reference image.

Adding gameplay elements from the reference image requires an explicit user request.

---

## Locked implementation (not to be changed for visual reasons)

- camera,
- map layout,
- gameplay scale,
- player silhouette,
- gameplay logic.

---

## Approved rendering baseline

This is the **project-wide** baseline, not a Mission 1-only snapshot. Once a visual element is approved in one mission, it is the default for every later mission — reuse the same asset rather than authoring an equivalent. See "Asset reuse across missions" below.

Measured from PIE at the point the direction was frozen. Treat these as the reference point for any future visual change.

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

Required ordering, darkest to brightest:

floor grid seam < floor panel < wall sides < wall tops < gameplay actors < player projectile (brightest).

### Materials backing this baseline

| Asset | Role |
| --- | --- |
| `M_HackUnlit` | generic unlit master (`BaseColor`, `Intensity`) |
| `M_HackWall` / `MI_Wall` | walls; top/side split driven by world-normal Z. `TopColor (0.95,0.96,1.0)`, `SideColor (0.085,0.088,0.10)`, `Intensity 1.6`, `SheenIntensity 0` |
| `M_Floor_Texture` + `floor` texture | floor; texture-driven panel/grid pattern sampled from the `floor` texture. Parameters: `Texture`, `Tiling`, `BaseColor`, `Brightness`, `Emissive`. Applied directly to the floor geometry — there is no material instance. Replaced an earlier procedural frac-based panel master (`M_HackFloor` / `MI_Floor`), which no longer exists; do not reintroduce it. |
| `M_HackTarget` / `MI_Target` | target; world-Z banding. `BaseColor (1.0,0.09,0.03)`, `Intensity 3.2` |
| `MI_Player` | cyan, `Intensity 3.0`, red channel pinned at 0 |
| `MI_ProjectilePlayer` | brightest element, `Intensity 4.0` |
| `MI_Exit` / `MI_ExitInactive` | objective active / inactive states |
| `MI_ShooterEnemy` | Mission 2 stationary shooter; `M_HackUnlit`, `BaseColor (1.0,0.06,0.03)`, `Intensity 3.4` — glowing red, clearly hostile |
| `MI_ProjectileEnemy` | Mission 2 enemy projectile; `M_HackUnlit`, `BaseColor (1.0,0.05,0.55)`, `Intensity 3.0` — magenta, distinct from the player's cyan-white shot |
| `MI_Barrier` | Mission 2 locked gate; `M_HackUnlit`, `BaseColor (1.0,0.08,0.05)`, `Intensity 1.8` — deeper/dimmer red than the shooters so it reads as architecture, not a target |
| `NS_PlayerTrail` / `M_HackTrail` | player movement trail; Niagara ribbon + sprite emitters (CPU sim) driven by `M_HackTrail` (Unlit, Translucent; `Color`, `Intensity`, `Opacity`). Attached to `BP_HackPlayerCharacter` as the `TrailFX` Niagara component. This is a motion-dependent effect — judge and tune it only from live play, never from a still frame. |
| `M_LockedGate` / `M_LockedGateGlow` | locked gate; striped hazard pattern on the barrier plus an additive glow pass. Used by `BP_MissionBarrier`, `BP_MissionBarrier_Side` and `BP_GateBeam` in Missions 2 and 3 |
| `MI_WallEnemyGray` | Mission 2 non-hostile destructible obstacle (reuses `BP_DestructibleTarget`); `M_HackUnlit`, `BaseColor (0.55,0.57,0.60)`, `Intensity 2.4` — neutral gray, applied as a per-instance override so `MI_Target`/Mission 1's target are untouched |

Post-process: exposure **locked** (auto-exposure min = max = 1.0, bias 0, physical camera exposure off), bloom 0.65, motion blur 0, vignette 0. Originally set on `L_Mission01`; confirmed copied verbatim to `L_Mission02`'s `PostProcessVolume` — this is the standard for every mission's PostProcessVolume, not a Mission 1 peculiarity. Apply the same 7 overrides to any new mission's volume before doing any other visual work on it, or the scene will read as washed out regardless of material values (see hard-won constraint 1).

The only light in the scene is `CyanPresenceLight`, a PointLightComponent on `BP_HackPlayerCharacter` (25 cd, colour (0.251, 0.851, 1.0), 250 cm attenuation, shadows off). It grounds the player with a small cyan pool on the floor. This is a player-attached light, so it is already present in every mission automatically — no per-mission setup needed.

---

## Asset reuse across missions

**If a visual solution has already been approved, reuse it by default. Do not reinvent it unless explicitly requested.**

1. Before creating or assigning a material, texture, effect, or environment visual for a new mission, check this document's "Materials backing this baseline" table and the previous mission's assets first. If an equivalent approved asset exists, reuse it (reparent, apply the existing instance, or copy exact values) rather than authoring a new one.
2. An approved visual becomes the baseline for every later mission the moment it is accepted, not just for the mission it was built in. Floor, walls, exposure/post-process, and any shared VFX all fall under this.
3. Never reintroduce an old placeholder, default, or previously replaced asset once a newer approved one exists for that purpose — including when building a new mission from scratch.
4. Do not create a near-duplicate of an existing approved material without a clear gameplay or art-direction reason (e.g. `MI_ShooterEnemy` and `MI_WallEnemyGray` are deliberately distinct because one must read as hostile and the other must not — that is a legitimate reason; a second red material for no functional reason is not).
5. Mission-specific visual differences should come from gameplay, layout, enemies, or an explicit user request — never from accidental drift in a shared material that should have been reused.
6. When a new mission-specific asset IS approved (a genuinely new element, not a reuse), add it to the "Materials backing this baseline" table above so the next mission checks against it too.

---

## Hard-won constraints — read before changing anything

These were each discovered the expensive way. Do not re-learn them.

1. **The exposure lock is load-bearing.** Unclamped auto-exposure re-normalises this deliberately dark scene to mid-grey, lifting every authored value roughly 30x and collapsing the entire value ladder. Three floor iterations were rejected while chasing this symptom before the cause was found. Never re-enable auto-exposure.

2. **Judge the rendered frame, never the parameter values.** Numeric ordering is not perceptual ordering. A grid measured "23% below the wall value" still dominated the screen, because thin emissive lines through a bloom pass read far brighter than a flat surface at the same numeric value.

3. **The floor material (`M_Floor_Texture`) is Default Lit, not Unlit.** Its dark base comes from an emissive term; its albedo still responds to lights. This is intentional — it is what lets the player's cyan light pool on the floor. But it also means *any* light added to a mission will light the floor. A lit-wall experiment did exactly that and lifted the floor from 12 to a 36-102 gradient, inverting the hierarchy.

4. **View-dependent effects do not work here.** The camera is locked at -60 degrees pitch, so wall tops are always viewed at a near-constant angle. A Fresnel sheen was tried and produced almost nothing, because there is no viewing-angle variation for it to key off. Prefer world-position or normal-driven treatments over view-driven ones.

5. **Wall tops sit near the tonemapper's highlight shoulder.** At ~212/255, further linear increases compress heavily in 8-bit output. Pushing brightness harder yields progressively less on screen and flattens tops toward uniform white.

6. **Brightness without desaturation requires reducing the non-dominant channels.** Colours wash to white/cream when multiple channels exceed 1.0 and clip together. To make a colour brighter *and* keep it saturated, raise `Intensity` and lower the non-dominant channels in `BaseColor` so only the dominant channel clips.

7. **`MI_Target`'s resting `Intensity` is coupled to gameplay feedback.** `ADestructibleTarget` drives `BaseColor`/`Intensity` on a dynamic material instance for its hit flash (9.0 on hit, 16.0 on death). Raising the resting value shrinks the flash headroom. Keep it at or below ~3.5.

8. **After editing a material graph, verify `MP_EmissiveColor` is still connected.** A disconnected emissive output renders the surface pure black and is easy to miss. This has happened.

9. **Save incrementally when editing materials.** Reparenting a material instance to a just-compiled master and immediately pushing parameter edits crashed the editor with a render-thread assertion. Compile -> save -> reparent -> save -> set parameters -> save.

---

## Process for any future visual task

1. **Check for an existing approved asset first.** Look at "Materials backing this baseline" above and at the previous mission's assets. If an equivalent already exists and is approved, reuse it directly and skip to step 5 — do not author a new material for something already solved.
2. If genuinely new, capture the current PIE state via MCP first, and treat that render as the structural baseline. Do not rely on remembered or reported state.
3. Modify only the requested element.
4. Compare the result against `concept_arena_reference.png`.
5. Delegate a read-only review to the Visual Reviewer, supplying both the previous accepted capture and the new one so regressions are detectable.
6. Stop for human review.
7. If a genuinely new asset was approved, add it to "Materials backing this baseline" so later missions check against it too.

Human visual acceptance is authoritative. If the human says a result looks worse, treat that as evidence requiring reconsideration rather than defending the implementation.

---

## Known gaps against the reference

Present in the approved concept but not yet implemented. Listed for tracking only — do not act on these without an explicit request.

- **Blocked-state colour language on the exit** — the concept expresses "blocked" as orange-red hazard striping. The striping itself now exists on the locked gate (`M_LockedGate`), but the inactive exit is still grey (`MI_ExitInactive`) rather than sharing that language.
- **Objective colour** — the concept's exit reads cyan; the current active exit is green.
