# Project state

Read this first in every session. Claude updates it at the end of every session.

- Updated: 2026-11-18
- Milestone: M0 Setup done (confirmed by you). Design review applied to the docs; next is M1 Spikes
- Last good commit: see `git log` on `main` (not tracked here)
- Branch: `feature/m0-setup` (docs changes from the design review are uncommitted; see `NEXT_STEPS.md`)

## How to run
Open `DemoCo.uproject` in Unreal Engine 5.8.3 (Launcher build). The MCP server listens on port 8000 (see `SETUP_NOTES.md`). Paste the prompt from `NEXT_STEPS.md` into Claude Desktop.

## Done
- Design decisions approved and recorded in `DECISIONS.md`; Appendix B defaults confirmed
- Design review (2026-11-18) applied: replication model, fracture pattern, rendering intent, milestone changes. Later same day: pattern revised to irregular baked cells with accepted aim offset (lattice kept as fallback). See the change log in `DECISIONS.md`
- Docs skeleton in `Docs/` (README, DECISIONS, ARCHITECTURE, SETUP_NOTES, PROGRESS, NEXT_STEPS)
- Day-0 items 1 to 6 and 8 pass; item 7 fallback by design. Details in `SETUP_NOTES.md`
- Git LFS installed and `.gitattributes` added by you

## Next
See `NEXT_STEPS.md`. In short: commit the docs, finish the LFS check, clean up the old test actor (needs your go), then M1 Spikes A, B and C.

## Broken or blocked
- Nothing blocking.

## Known leftovers
- `Source/DemoCo/Destruction_structure/` (`DestructionTestActor.h/.cpp`) is an old script and is to be removed. `Content/Destruction/BP_DestructionTestActor` derives from it and is referenced by one placed actor in `Lvl_FirstPerson` (external actor file `/Game/__ExternalActors__/FirstPerson/Lvl_FirstPerson/A/51/GMOKKWU7H1980ID08AXZ2R`). Not removed yet; see `NEXT_STEPS.md`.
- `DemoCoCharacter` contains a prototype vault (`VaultCheck`, `Server_VaultTo`). It is untested, allows up to 200 cm (the design says about 120 cm), uses a server-only `LaunchCharacter` that will rubber-band remote clients, and only traces WorldStatic and WorldDynamic. It is replaced in M3.
- `DemoCoGameMode.cpp` contains a template-era stub log line (`Day0 LiveCoding test`). The project's default game mode is still the template's `BP_ShooterGameMode`. Left as found.
- Template rendering config is still on (Lumen, hardware ray tracing, Substrate, virtual shadow maps). Reviewed in Spike C; changes need your approval.
- `Build.cs` still lists `GeometryCollectionEngine` and `FieldSystemEngine` for the old test actor. Keep until Spike A is done.
- Template folders `Variant_Horror` and `Variant_Shooter` are unused by the design (no AI). Optional cleanup later.

## Open decisions
- Whether to remove the old destruction test actor on its own branch (see `NEXT_STEPS.md`)
- Spike results decide: rendering representation, irregular-cell hierarchy versus lattice fallback (look and bakeability), Chaos versus custom for group simulation
