# Next steps and session prompt

Updated: 2026-11-18. Claude rewrites the "Next steps" section at the end of every session. `PROJECT_STATE.md` holds status; this file holds the plan and the prompt to paste.

## Next steps

### To close M0
1. **Done:** branch `feature/m0-setup` created and pushed with the docs.
2. **Done:** Day-0 item 3 passed. A function-body edit compiled through `CompileLiveCoding` (`Success`). The test edit was reverted, so `Source/` is unchanged.
3. **Optional, needs approval:** change editor Play settings to 3 to 4 players (listen server) so Day-0 item 6 covers the full 4-player case. Currently verified with 2 players. Can also be done at the start of M1 when the multiplayer spike needs it.
4. **You:** commit the updated `Docs/` on `feature/m0-setup`. Suggested message: `M0: Day-0 item 3 passes, docs updated`. Merge to `main` after review.
5. Claude marks M0 closed in `PROJECT_STATE.md` once `main` has the merge (the state file already says it is waiting for that).

### Cleanup: remove the old destruction test actor (needs your "go")
You approved removing the old `Destruction_structure` script. It touches assets and a level, so Claude waits for "go" before doing the Unreal side. Suggested: do it on its own branch after M0 is merged, `feature/cleanup-old-destruction-test`, so the M0 commit stays docs only.

What is involved (found by checking references):
- `Source/DemoCo/Destruction_structure/DestructionTestActor.h` and `.cpp`
- `Content/Destruction/BP_DestructionTestActor.uasset`, a Blueprint of that class
- One instance of that Blueprint placed in `Lvl_FirstPerson` (external actor file `__ExternalActors__/FirstPerson/Lvl_FirstPerson/A/51/GMOKKWU7H1980ID08AXZ2R`)

Order (the C++ goes last so the editor never loads a Blueprint with a missing parent):
1. **You:** create the branch. Save everything in the editor.
2. **Claude, after "go":** delete the placed actor from `Lvl_FirstPerson` and save the level; delete `BP_DestructionTestActor`; confirm with `get_referencers` that nothing references it any more.
3. **You:** close the editor and delete the C++ files with Git: `git rm -r Source/DemoCo/Destruction_structure`. (Claude has no delete tool for source files.)
4. **You:** rebuild `Development Editor` from Visual Studio. Removing a class is not a Live Coding change. A compile error would mean something else still includes the header; tell Claude the output.
5. Reopen the editor. Claude checks with `list_toolsets` and a read-only call, then reads `LogsToolset` for missing-class warnings when the level loads. Pass = clean build, no warnings, level loads.
6. **You:** commit. Suggested message: `Remove old destruction test actor`.

The empty `Content/Destruction/` folder stays; it is the planned content folder in `DEV_WORKFLOW_MCP.md` section 5.

### Then M1 (Spikes)
- **Spike A:** one wall, listen server plus 2 clients. Compare Chaos Geometry Collection replication with custom chunk events (divergence and bandwidth).
- **Spike B:** can fracture/Dataflow or Geometry Script be driven from editor code?
- Set performance budgets.
- Done when a decision for runtime and baker approach is recorded in `DECISIONS.md`.
- First session of M1: Claude proposes the plan for Spike A (files, test, expected result) and waits for "go". Branch `feature/m1-spike-a` is created by you.
- Note for Spike A: the removed script used Chaos field strain on a Geometry Collection, which is the "Chaos Geometry Collection" side of the comparison. Its approach can be reread from git history if useful.

## Prompt for Claude Desktop

Paste this at the start of every new chat. Prerequisites: editor open on `DemoCo.uproject`, MCP server running (port 8000), filesystem connector scoped to `E:\unity\projects\DemoCo`.

```
Project folder: E:\unity\projects\DemoCo (UE 5.8.3, C++). MCP server: http://127.0.0.1:8000/mcp.
Read Docs/PROJECT_STATE.md, Docs/NEXT_STEPS.md, Docs/DECISIONS.md and Docs/DEV_WORKFLOW_MCP.md.
Confirm the editor is reachable with list_toolsets. Then tell me the current milestone, the next task,
the plan for this session in a few lines, and the test that will prove it.
Do not change anything until I say go.
Rules: follow the approval rules in section 10 of DEV_WORKFLOW_MCP.md; keep changes small; write all docs
into Docs/; at the end of the session update PROJECT_STATE.md, NEXT_STEPS.md and PROGRESS.md and give me
the commit message and Git commands (I run Git).
```

Notes for the prompt:
- To start a specific task, add one line: `Task for this session: <task from Next steps>.`
- `call_tool` takes the bare tool name (for example `get_current_level`), not the prefixed one shown by `describe_toolset`.
- `AssetTools.read_file/write_file` only reach `Content/` and `Saved/`; use the filesystem connector for `Docs/` and `Source/`.
- Claude can edit and move files through the filesystem connector but cannot delete them; deletions of source files are done by you with Git.
- If `CompileLiveCoding` says `NoChanges` after a real edit, see the quirks in `SETUP_NOTES.md`.
