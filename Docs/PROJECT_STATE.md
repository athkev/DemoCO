# Project state

Read this first in every session. Claude updates it at the end of every session.

- Updated: 2026-11-18
- Milestone: M0 Setup (in progress, one check left)
- Last good commit: none yet
- Branch: `main` currently checked out (plan says `feature/m0-setup`; see Next)

## How to run
Open `DemoCo.uproject` in Unreal Engine 5.8.3 (Launcher build). The MCP server listens on port 8000 (see `SETUP_NOTES.md`). Paste the prompt from `NEXT_STEPS.md` into Claude Desktop.

## Done
- Design decisions approved and recorded in `DECISIONS.md`; Appendix B defaults confirmed
- Docs skeleton in `Docs/` (README, DECISIONS, ARCHITECTURE, SETUP_NOTES, PROGRESS, NEXT_STEPS)
- Day-0 items 1, 2, 4, 5, 6, 8 pass; item 7 fallback by design; item 3 partial (toolset works, `NoChanges` baseline compile ok). Details in `SETUP_NOTES.md`

## Next
See `NEXT_STEPS.md`. In short: you create branch `feature/m0-setup`, Claude does the trivial-change Live Coding compile (item 3), then commit and close M0.

## Broken or blocked
- Item 3 trivial-change compile waits for a feature branch (source edits on `main` are not allowed by the approval rules)

## Open decisions
- None
