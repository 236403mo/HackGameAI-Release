# Third-Party Notices

This repository contains a modified version of the **Thermo-Nuclear Code Quality Review** skill originally distributed as part of the **Cursor Team Kit** in the `cursor/plugins` repository.

## Thermo-Nuclear Code Quality Review

- **Original project:** Cursor Team Kit
- **Original component:** `thermo-nuclear-code-quality-review`
- **Copyright:** Copyright (c) 2026 Cursor
- **License:** MIT License

The version included in this repository has been adapted for use with **Unreal Engine and C++**, including changes to the original review instructions and project-specific guidance.

These modifications are not part of the original Cursor Team Kit distribution and should not be interpreted as an official Cursor version of the skill.

The original component is distributed under the MIT License. A copy of the applicable MIT License must be retained with the redistributed or modified component; it is included in this repository at [`.claude/skills/thermonuclear-code-review/LICENSE.txt`](.claude/skills/thermonuclear-code-review/LICENSE.txt).

All other project-specific content in this repository is separate from the third-party component described above.

## Audio Assets

The sound effects used during development are third-party assets. They are intentionally **not** distributed in this repository, and are excluded from version control via `.gitignore` (`/Content/HackGameAI/Sounds/`).

Some Blueprint properties still reference those locally installed assets:

- `BP_HackPlayerCharacter` — `/Game/HackGameAI/Sounds/SFX_Player_Shot`
- `BP_ShooterEnemy` — `SFX_Enemy_Shot`, `SFX_Enemy_Damage`, `SFX_Enemy_Destroyed`
- `BP_MovingShooterEnemy` — `SFX_Enemy_Shot`, `SFX_Enemy_Damage`, `SFX_Enemy_Destroyed`
- `BP_CaterpillarSegment` — `SFX_Enemy_Shot`, `SFX_Enemy_Damage`, `SFX_Enemy_Destroyed`
- `BP_DestructibleTarget` — `SFX_Enemy_Damage`, `SFX_Enemy_Destroyed`

When cloning this repository, expect these references to be unresolved. The project is expected to run without those sounds: the sound properties are optional and each playback call is guarded, so the game runs silently unless you supply your own replacement assets at the paths listed above.
