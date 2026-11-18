# Next steps and session prompt

Updated: 2026-11-18. Claude rewrites the "Next steps" section at the end of every session. `PROJECT_STATE.md` holds status; this file holds the plan and the prompt to paste.

## Next steps

### 1. Commit the design-review docs (you)
The docs changed on `feature/m0-setup` (DECISIONS, ARCHITECTURE, DEV_WORKFLOW_MCP, PROJECT_STATE, NEXT_STEPS, PROGRESS, SETUP_NOTES). M0 is done, so put this on its own branch; uncommitted changes carry over:
```
git switch -c docs/design-review
git add Docs
git commit -m "Docs: apply design review (replication model, fracture pattern, milestones)"
```
Review the diff first. Merge to `main` after review.

### 2. Finish the LFS setup (you, quick)
- Run `git lfs ls-files`. Any `.uasset` or `.umap` committed before LFS was set up is still in normal history; if some are, tell Claude, and the fix is `git lfs migrate import --include="*.uasset,*.umap"` (rewrites history, so do it before sharing the branch further).
- Consider adding to `.gitattributes`: `*.exr`, `*.jpg`, `*.jpeg`, `*.ogg`, `*.ttf`, `*.otf`, `*.blend`, `*.wem` if you use those.
- Consider adding to `.gitignore`: `.idea/`, `PackageBuild/`, `*.user`.

### 3. Cleanup: remove the old destruction test actor (needs your "go")
You approved removing the old `Destruction_structure` script. It touches assets and a level, so Claude waits for "go" before doing the Unreal side. Do it on its own branch, `feature/cleanup-old-destruction-test`, after the docs are merged.

What is involved (found by checking references):
- `Source/DemoCo/Destruction_structure/DestructionTestActor.h` and `.cpp`
- `Content/Destruction/BP_DestructionTestActor.uasset`, a Blueprint of that class
- One instance of that Blueprint placed in `Lvl_FirstPerson` (external actor file `__ExternalActors__/FirstPerson/Lvl_FirstPerson/A/51/GMOKKWU7H1980ID08AXZ2R`)

Order (the C++ goes last so the editor never loads a Blueprint with a missing parent):
1. **You:** create the branch and commit first (rollback point). Save everything in the editor.
2. **Claude, after "go":** delete the placed actor from `Lvl_FirstPerson` and save the level; delete `BP_DestructionTestActor`; confirm with `get_referencers` that nothing references it any more.
3. **You:** close the editor and delete the C++ files with Git: `git rm -r Source/DemoCo/Destruction_structure`. (Claude has no delete tool for source files.)
4. **You:** rebuild `Development Editor` from Visual Studio. Removing a class is not a Live Coding change.
5. Reopen the editor. Claude checks with `list_toolsets` and a read-only call, then reads `LogsToolset` for missing-class warnings when the level loads. Pass = clean build, no warnings, level loads.
6. **You:** commit. Suggested message: `Remove old destruction test actor`.

`Build.cs` keeps the Geometry Collection and Field System dependencies until Spike A is done.

### 4. M1 Spikes
Branches: `feature/m1-spike-a`, `-b`, `-c`, created by you. Claude proposes the plan for each (files, test, expected result) and waits for "go".
- **Spike C first (rendering and cost):** record a baseline on a mid-range-style setup (Lumen, ray tracing, virtual shadow maps as they are now), then test about 100k pieces for intact buildings (instanced meshes with hidden instances versus swapped proxies) and the cost of awake groups. Config changes are proposed to you, not applied.
- **Spike B (baker):** can Geometry Script or fracture/Dataflow be driven from editor code to bake an irregular-cell hierarchy (leaves about 1 m, adjacency stored, aim offset within about 0.5 m), and does it look like The Finals. Compare with the lattice fallback (1 m leaves, jagged cuts). May need enabling a plugin, which needs your approval.
- **Spike A (runtime):** one wall, listen server plus 2 clients. Custom state and group replication is the baseline; compare Chaos Geometry Collection for one day. The removed script used Chaos field strain on a Geometry Collection and can be reread from git history.
- Set budgets: pieces, awake groups, bytes per second.
- Done when `DECISIONS.md` records the runtime, rendering representation and baker approach.
- Optional, needs approval: set editor Play settings to 3 to 4 players (listen server) so Day-0 item 6 covers the full 4-player case.

## Prompt for Claude Desktop

Paste this at the start of every new chat. Prerequisites: editor open on `DemoCo.uproject`, MCP server running (port 8000), filesystem connector scoped to `E:\unity\projects\DemoCo`.

```
Project folder: E:\unity\projects\DemoCo (UE 5.8.3, C++). MCP server: http://127.0.0.1:8000/mcp.
Read CLAUDE.md, Docs/PROJECT_STATE.md, Docs/NEXT_STEPS.md, Docs/DECISIONS.md and Docs/DEV_WORKFLOW_MCP.md.
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
