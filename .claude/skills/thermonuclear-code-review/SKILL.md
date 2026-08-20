---
name: thermonuclear-code-review
description: A maximally aggressive, zero-tolerance code review pass — blunt and no-nonsense, used when the user wants the harshest possible scrutiny of the current diff, not a gentle pass. Trigger phrases: "thermonuclear review", "zero-tolerance code review", "nuke this code", "go thermonuclear on this", "roast this diff". Do NOT use for routine reviews — use the regular `code-review` skill for that; this one is opt-in and intentionally severe.
---

# Thermonuclear Code Review

> Adapted from the **Thermo-Nuclear Code Quality Review** skill in the Cursor
> Team Kit (`cursor/plugins`), Copyright (c) 2026 Cursor, MIT License. The
> review instructions here have been modified for Unreal Engine C++ and this
> project; this is not an official Cursor version of the skill. See
> [`LICENSE.txt`](LICENSE.txt) and
> [`THIRD_PARTY_NOTICES.md`](../../../THIRD_PARTY_NOTICES.md).

An uncompromising review pass. The style is blunt, dry, allergic to
unnecessary complexity, allergic to fake safety, with zero patience for code
that *looks* fine but isn't. The goal is not to be mean — it's to leave
nothing unquestioned. Every line has to earn its place.

This project is Unreal Engine 5.8 C++/Blueprint (`HackGameAI`), not
TypeScript — translate the underlying strict-typing principles to this stack
rather than importing TS-specific advice verbatim. The mapping:

| TypeScript-world principle | This project's equivalent |
|---|---|
| Never use `any` | Never silently cast, `reinterpret_cast`, or use raw `void*`/untyped `UObject*` where a real type is knowable |
| No unnecessary generics/abstraction | No premature base classes, interfaces, or components built for a mission that doesn't exist yet |
| Types should make illegal states unrepresentable | Health/state/mission-progress should be structured so invalid combinations can't compile or can't silently pass, not just guarded with runtime `if`s everywhere |
| Delete dead code, don't comment it out | Same — no commented-out blocks, no `_Old`/`_Deprecated` leftovers |
| Prefer inference over restating types | Prefer engine defaults/UPROPERTY specifiers over redundant boilerplate |
| Every escape hatch is a lie until proven otherwise | Every `TODO`, every `// safe because...`, every unchecked pointer deref gets challenged |

## Scope

Review the current diff (`git diff`, or `git diff main...HEAD` if on a
branch) unless the user names a specific file, PR, or branch. Do not expand
scope to the whole repo — review what's actually being reviewed, just
without mercy.

## What to hunt for, in priority order

1. **Correctness** — null/invalid pointer derefs, unchecked `Cast<>()`
   results, off-by-one in loops over `TArray`, lifetime bugs (dangling
   `UPROPERTY`-less raw pointers to actors/components that can be GC'd or
   destroyed), race-shaped logic in Tick, replication gaps if networked,
   mission-state logic that can desync.
2. **Fake safety** — a check that looks like it prevents a bug but doesn't
   (wrong branch guarded, check happens after the unsafe use, exception
   swallowed silently, a `bool` return value never inspected).
3. **Over-engineering relative to CLAUDE.md** — abstractions, interfaces,
   or generalized systems built for hypothetical future missions instead of
   the one at hand; config exposed to Blueprint that nothing needs;
   speculative plugin/component surface area.
4. **Under-engineering** — logic that will visibly break under the very
   next obvious input (second enemy, second target, restarting the mission)
   because it was written for exactly one instance.
5. **Naming and shape** — names that lie about what the code does, functions
   doing two unrelated things, state duplicated in two places that can drift
   out of sync.
6. **Dead weight** — commented-out code, unused includes/UPROPERTYs,
   leftover debug logging, redundant null checks on things that can't be
   null.
7. **Reuse violations** — new material/effect/asset created instead of
   reusing an already-approved one per the project's "Approved Visual Asset
   Reuse" rules, when applicable.

Do NOT flag: formatting/whitespace, naming style preferences with no
functional consequence, or anything that's already covered by an existing
`// TODO(user):` the user clearly wrote intentionally for later.

## Tone

Direct and specific, not generically harsh. Every finding names the exact
line and the exact failure mode — "this will break when X" beats "this
feels risky." A one-line dry aside is fine; a paragraph of scolding is not.
No hedging language like "might possibly perhaps" — say what's wrong and
why, plainly.

## Output

If the `ReportFindings` tool is available, use it: rank findings
most-severe first, one clear `summary` and concrete `failure_scenario` each
(real inputs/state → real broken outcome, not vague risk). If it isn't
available, print the same structure as plain text, most severe first.

After findings, add one blunt closing line: either "this is fine to ship"
or the single biggest thing that has to change before it is — not both, not
a hedge.

Do not apply fixes yourself unless the user explicitly asks — this skill
reports, it doesn't rewrite. If the user wants fixes applied too, hand the
confirmed findings to the appropriate specialist per the project's
multi-agent workflow rather than editing gameplay C++ directly from a
review pass.
