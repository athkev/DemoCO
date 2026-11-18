# Project state

Read this first in every session. Claude updates it at the end of every session.

- Updated: 2026-11-18
- Milestone: M0 Setup, Day-0 checks complete; waiting for the final commit and merge to `main`
- Last good commit: none yet on `main`
- Branch: `feature/m0-setup`

## How to run
Open `DemoCo.uproject` in Unreal Engine 5.8.3 (Launcher build). The MCP server listens on port 8000 (see `SETUP_NOTES.md`). Paste the prompt from `NEXT_STEPS.md` into Claude Desktop.

## Done
- Design decisions approved and recorded in `DECISIONS.md`; Appendix B defaults confirmed
- Docs skeleton in `Docs/` (README, DECISIONS, ARCHITECTURE, SETUP_NOTES, PROGRESS, NEXT_STEPS)
- Day-0 items 1 to 6 and 8 pass; item 7 fallback by design. Item 3 passed on `feature/m0-setup` (Live Coding compile returned `Success` after a Visual Studio build made `DemoCo.dll` available). Item 6 is verified with 2 players. Details in `SETUP_NOTES.md`

## Next
See `NEXT_STEPS.md`. In short: you commit the docs and merge M0 to `main`, then the old `Destruction_structure` cleanup (needs your go), then M1 Spike A.

## Broken or blocked
- Nothing blocking.

## Known leftovers
- `Source/DemoCo/Destruction_structure/` (`DestructionTestActor.h/.cpp`) is an old script and is to be removed. `Content/Destruction/BP_DestructionTestActor` derives from it and is referenced by one placed actor in `Lvl_FirstPerson` (external actor file `/Game/__ExternalActors__/FirstPerson/Lvl_FirstPerson/A/51/GMOKKWU7H1980ID08AXZ2R`). Not removed yet; see `NEXT_STEPS.md`.
- `DemoCoGameMode.cpp` contains a template-era stub log line (`Day0 LiveCoding test`). Left as found.

## Open decisions
- Whether to remove the old destruction test actor on `feature/m0-setup` or on its own branch (see `NEXT_STEPS.md`)
