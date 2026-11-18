# Next steps and session prompt

Updated: 2026-11-18. Claude rewrites the "Next steps" section at the end of every session. `PROJECT_STATE.md` holds status; this file holds the plan and the prompt to paste.

## Next steps

### To close M0
1. **You:** create the branch: `git checkout -b feature/m0-setup` (repo is on `main`).
2. **Day-0 item 3:** Claude makes a trivial function-body edit in `Source/DemoCo` and calls `LiveCodingToolset.CompileLiveCoding`. Pass = explicit success with a compile (not `NoChanges`). The toolset itself is already confirmed working (baseline returned `NoChanges`).
3. **Optional, needs approval:** change editor Play settings to 3 to 4 players (listen server) so Day-0 item 6 covers the full 4-player case. Currently verified with 2 players.
4. **You:** commit `Docs/` on `feature/m0-setup`. Suggested message: `M0: docs skeleton and Day-0 results`. Merge to `main` after review.
5. Claude marks M0 done in `PROJECT_STATE.md`.

### Then M1 (Spikes)
- **Spike A:** one wall, listen server plus 2 clients. Compare Chaos Geometry Collection replication with custom chunk events (divergence and bandwidth).
- **Spike B:** can fracture/Dataflow or Geometry Script be driven from editor code?
- Set performance budgets.
- Done when a decision for runtime and baker approach is recorded in `DECISIONS.md`.
- First session of M1: Claude proposes the plan for Spike A (files, test, expected result) and waits for "go". Branch `feature/m1-spike-a` is created by you.

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
