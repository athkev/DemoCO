# Development workflow: Unreal MCP + Claude Desktop

Project: co-op (up to 4 players) multiplayer destruction framework, Unreal Engine 5.8.3 (Launcher build), C++.
Document version: 1 (2026-11-18). Lives in the repo at `Docs/DEV_WORKFLOW_MCP.md` so every Claude session can reload it.

## 1. Purpose

This document defines how the project is built with Claude Desktop connected to the Unreal Editor through Epic's official Unreal MCP plugin. It covers:

- how the connection works and how to verify it (Day-0 checklist)
- who does what (Claude, you, the editor)
- the session protocol, since Claude does not remember earlier chats
- the build, test and approval loop
- the milestone plan and what Claude does with MCP in each milestone

What is verified and what is not is listed in section 11. Anything marked **VERIFY** must be confirmed on your machine before relying on it.

## 2. How the pieces connect

- The editor runs two plugins: `ModelContextProtocol` (server and transport) and `AllToolsets` (the tools). With only the first enabled the server starts but exposes no tools.
- The server listens on `http://127.0.0.1:8000/mcp` by default. It auto-starts if `EditorPerProjectUserSettings.ini` contains `bAutoStartServer=True` under `[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]`. Manual start: console command `ModelContextProtocol.StartServer`.
- Tool search is on by default, so the client sees only three meta-tools: `list_toolsets`, `describe_toolset`, `call_tool`. Claude discovers a toolset with `describe_toolset`, then invokes tools through `call_tool` with `toolset_name`, `tool_name` and `arguments`.
- Optional STDIO proxy (`Engine/Plugins/Experimental/ModelContextProtocol/Extras/Proxy`, binary `Bin/Win64/unreal_mcp_proxy.exe` on Windows): keeps the client session alive while the editor is closed and reopened. Recommended, because this project restarts the editor often (header changes, new modules). It is also the STDIO entry that desktop clients are most likely to accept. **VERIFY** against your current Claude Desktop config.
- The plugin's `ModelContextProtocol.GenerateClientConfig` writes configs for Claude Code, Cursor, VS Code, Gemini and Codex, not Claude Desktop. Your Claude Desktop entry is therefore hand-maintained. Record the working entry in `Docs/SETUP_NOTES.md` (Day-0 item 8).
- After any plugin change, `ModelContextProtocol.RefreshTools` re-registers toolsets. Verify recovery after an editor restart with a read-only tool call, not with the proxy status (the status only says the proxy holds a session, not that Unreal is reachable).

## 3. Roles

| Who | Does |
|---|---|
| Claude (Desktop) | Reads and writes C++ and docs in the repo, drives the editor through MCP, runs compiles and automation tests, reads results, proposes commits |
| You | Decides, reviews every diff, runs Git, rebuilds from Rider/Visual Studio when needed, plays and judges feel, approves risky actions (section 9) |
| Editor | Source of truth for assets, PIE results, Output Log |

## 4. Day-0 checklist (milestone M0)

Do these in order. Each one confirms a capability this document assumes. If one fails, the fallback applies and this document gets updated.

| # | Check | How | If it fails |
|---|---|---|---|
| 1 | Server reachable | Claude calls `list_toolsets` | Run `ModelContextProtocol.StartServer`; check both plugins are enabled in the `.uproject`; run `RefreshTools`; check the port (override with `ServerPortNumber`) |
| 2 | Read-only call works | Claude calls `describe_toolset` on a level or asset toolset, then one read-only tool | Wait if the editor is compiling, loading or in PIE |
| 3 | C++ compile through MCP | `LiveCodingToolset.CompileLiveCoding` on a trivial change. It blocks until the compile finishes and returns MSVC diagnostics | Compile from Rider/Visual Studio and tell Claude the result |
| 4 | Automation tests through MCP | `AutomationTestToolset`: `DiscoverTests`, then `ListTests`, `RunTests`, `GetTestStatus`, `GetTestResults`. `DiscoverTests` must come first or results are empty | Command line: `UnrealEditor-Cmd.exe <Project>.uproject -ExecCmds="Automation RunTests <Filter>;quit" -Unattended -NullRHI` |
| 5 | Source file access | Claude lists toolsets for a file or source toolset **VERIFY**. If none, connect a filesystem connector in Claude Desktop scoped to the repo folder only | Without file access you paste files and apply patches manually (works, slower) |
| 6 | Multi-client PIE | Claude (or you) starts PIE as listen server with 2+ clients; Claude reads the Output Log **VERIFY** | You start PIE; you paste log excerpts |
| 7 | Git | Claude has no Git tool by default. You run Git; Claude writes the commands and commit messages | none needed |
| 8 | Record the setup | Save the working Claude Desktop MCP entry and port in `Docs/SETUP_NOTES.md` | none |

M0 is done when items 1 to 4 pass, and 5 to 8 each have a pass or a documented fallback.

## 5. Repository and docs layout

```
<Project>/
  Docs/
    README.md              index of the docs
    PROJECT_STATE.md       current milestone, open tasks, last good commit, how to run (Claude reads this first)
    DECISIONS.md           approved decisions (Appendix A) plus a dated log of changes
    ARCHITECTURE.md        modules, data flow, replication design
    DEV_WORKFLOW_MCP.md    this file
    SETUP_NOTES.md         working MCP config, ports, known quirks
    PROGRESS.md            per-step "what changed and why" notes (learning log)
  Source/
    DestructionSolver/     pure C++, no UObject: fracture tree, connectivity graph, strain solver (unit-testable)
    DestructionCore/       runtime types, destructible asset, structure actor, event pipeline, replication
    DestructionGameplay/   tools, props, glass, debris, vehicles, Demolition mode
    DestructionEditor/     baker, validation, project MCP toolset (editor-only)
  Content/Destruction/{Materials,Buildings,Props,Tools,Vehicles,Modes}/
```

Conventions: Unreal prefixes (`SM_`, `BP_`, `DA_`, `M_`, `MI_`, `NS_`), one class per file, logic in C++, Blueprints only for thin variants, tuning values in data assets.

## 6. Session protocol

Claude has no memory between chats. The repo is the memory.

**Start of every session**, Claude:
1. reads `Docs/PROJECT_STATE.md`, then `DECISIONS.md`, then the architecture section for the current milestone
2. runs `list_toolsets` to confirm the editor is reachable
3. states the plan for this session in a few lines and waits for your go-ahead on anything past reading

**End of every session**, Claude:
1. updates `PROJECT_STATE.md` (what is done, what is next, anything broken)
2. adds a short entry to `PROGRESS.md`: what changed and why, in plain language, with the new Unreal concepts introduced
3. proposes the commit message and the exact Git commands for you to run

Starter prompt to paste in a new chat is in Appendix C.

## 7. The development loop

For every change:

1. **Plan.** Claude states the goal, the files to touch, and the test that will prove it. Changes stay small (aim for under about 200 lines) so you can review them.
2. **Branch.** You create `feature/<milestone>-<topic>` before work starts.
3. **Implement.** Claude edits C++ and data in the branch.
4. **Compile.** Function-body edits: `LiveCodingToolset.CompileLiveCoding`. New or changed `UCLASS`, `USTRUCT`, `UPROPERTY`, `UFUNCTION`, new modules and `.Build.cs` edits: close the editor and rebuild from Rider/Visual Studio. Treat this as the safe default; **VERIFY** what Live Coding handles in 5.8.3. The STDIO proxy keeps Claude's session alive across the restart.
5. **Test.** Run the automation tests for the area (section 8). Claude reads the result before moving on and treats anything that is not an explicit success as a stop.
6. **Review.** You read the diff. Claude explains anything new in plain language (the learning log in `PROGRESS.md`).
7. **Commit.** You run Git with Claude's message. Merge to `main` only after review.

### MCP safety rules (from the plugin's own guidance)

- Save the project before any bulk or multi-asset change, and again after. MCP edits are not always undoable, especially across compilation boundaries.
- Never call tools while C++ or shader compilation is running; they hang or fail confusingly.
- Serialize dependent calls and anything that mutates the same asset. Parallelize only independent, read-only work.
- Always read the result of a call. Success and failure can come back without an exception.
- Editor-only tools behave differently during PIE. If a result looks wrong, check whether PIE is running.
- One Claude client per editor at a time.

### Where each kind of change lives

| Change | Where | Why |
|---|---|---|
| Gameplay and replication logic | C++ | Diffable, testable, reviewable |
| Tuning values (health, damage multipliers, caps) | Data assets | Changeable without a rebuild |
| Variants of tools, props, vehicles | Blueprint subclasses of C++ bases | Fast iteration, small binary diffs |
| Level layout, asset setup | Editor, driven by Claude through MCP | Binary files, so commit after every session |

## 8. Testing strategy

1. **Solver unit tests (headless).** `DestructionSolver` has no engine dependencies, so Claude writes Automation tests for the fracture tree, connectivity, strain and detach logic and runs them through MCP. Every solver change ships with a test.
2. **Determinism and sync checks.** A debug command (for example `Destruction.DumpHash`) prints a hash of each structure's alive-piece bitset. After each scenario, the host and every client must print the same hash.
3. **Multiplayer matrix.** Listen server plus 1 to 3 clients, with Network Emulation profiles (good, average, bad), and a late-join case after a collapse. Checks: hashes match, rubble sits in the same place, no player gets stuck, bandwidth stays within budget.
4. **Performance.** `stat unit` and Unreal Insights on a mid-range PC as the host. Target is 60 fps with the host also simulating. Budgets are set in M1 and checked in every milestone from M4.
5. **Scenario scripts.** Fixed damage scenarios (a rocket at a point, C4 on a column) so results are repeatable. Once the project toolset exists (section 9), Claude runs these itself.

## 9. Project toolset and Agent Skills (from M2)

The plugin lets a project register its own toolsets (`UToolsetDefinition` with static `UFUNCTION(meta = (AICallable))` methods, registered with `ToolsetRegistry`) and its own Agent Skills (reached through `AgentSkillToolset.ListSkills` and `GetSkills`). Both are experimental in 5.8, so the API may shift. The plugin's guidance prefers Python when the Python API covers what the tool needs, and C++ otherwise. Tests are mandatory for every tool.

Planned `DestructionToolset` (in `DestructionEditor`):

- `ValidateBuilding(source)`: reports non-watertight meshes, unmapped materials, oversize root pieces
- `BakeBuilding(source, settings)`: one-click bake that writes the destructible asset and a report
- `GetStructureStats(structure)`: piece counts, groups, rubble count, estimated replication bytes
- `ApplyTestDamage(structure, location, radius, damage)` and `ResetStructure(structure)`: repeatable scenarios
- `GetStructureHash(structure)`: for the sync check in section 8

Planned project Agent Skills (written once the workflow exists, so they capture real steps): "bake a building", "add a destruction tool", "add a material", "run the multiplayer test matrix".

## 10. Approval rules

Claude may, on a feature branch: edit source and docs, create new files, compile, run tests, and run read-only or additive MCP calls.

Claude asks first before:
- deleting or renaming any asset or file (references break)
- changing project settings, `Config/*.ini`, or enabling or disabling plugins
- any bulk operation over many assets
- anything that touches `main`, commits, merges or pushes (you do these)
- adding a third-party plugin or dependency

## 11. What is verified and what is not

Verified from the plugin's own documentation: the three meta-tools and the `call_tool` flow; the two required plugins; auto-start and port settings; `LiveCodingToolset.CompileLiveCoding`; `AutomationTestToolset` workflow; Agent Skills and custom toolsets; the STDIO proxy; the safety rules in section 7.

Not verified, so checked in M0: Claude Desktop's exact config for this server; whether a file-editing tool exists; Output Log reading; multi-client PIE launch; what Live Coding handles in 5.8.3; whether your Launcher build ships the proxy.

## 12. Milestones

| M | Goal | Claude does (with MCP) | Done when |
|---|---|---|---|
| M0 | Setup | Day-0 checklist; create the C++ First Person project; write the `Docs/` skeleton | Section 4 passes |
| M1 | Spikes | **Spike A:** one wall, listen server plus 2 clients; compare Chaos Geometry Collection replication with custom chunk events (divergence and bandwidth). **Spike B:** can fracture/Dataflow or Geometry Script be driven from editor code. Set budgets | A decision recorded in `DECISIONS.md` for runtime and baker approach |
| M2 | Runtime core | Solver on a code-generated test structure; fracture tree; event pipeline; replicated bitset; late-join snapshot; project toolset v1 | Hashes match across host and clients in the test matrix |
| M3 | Player | Replace the template character with a replicated one on CharacterMovement; auto-step, hold-space vault | Two clients move and vault in a listen session |
| M4 | Tools | Sledgehammer, RPG, C4 as `FDestructionImpulse` sources; damage tables per material | Each tool breaks a test structure correctly on all clients |
| M5 | Debris and rubble | Detached groups as rigid bodies; settle to non-simulating rubble; **sync in groups, whole-structure snapshot as the extreme case**; lift-and-damage when debris lands on a player; falling groups damage other structures | Collapse of a multi-floor test building stays in sync and holds frame rate |
| M6 | Baker | One-click bake per the approved rules; validation report; Physical Material then keyword mapping | A real artist-made building bakes and plays correctly |
| M7 | Props, glass, mode | Props with carry and strength; valuables; glass panes; Demolition mode, HUD percentage, timer, round reset | A full round plays start to finish with 2 clients |
| M8 | Vehicles | Crane (physical ball), then bulldozer, then truck; driver-simulated; riders on seat slots | Each vehicle damages structures consistently for all players |
| M9 | Scale and ship | 20 to 30 buildings, profiling, budgets, Steam sessions (engine Steam plugins), packaging | 60 fps on a mid-range host in a 4-player session |

Rubble rule (your D7 amendment): settled debris becomes non-simulating rubble and is synchronized in groups. Old rubble is not removed. A cap on live pieces is added only if M5 or M9 profiling shows the group-sync approach is not enough, and then it is the last resort.

Stretch, after M9: gradual sag (the second layer in The Finals' integrity system), host migration, streaming.

## Appendix A: Approved decisions (summary)

- Fracture tree from root to 1 m leaves, size halving per level, max depth 4 (root piece cap about 16 m; larger structures are split into several root pieces by the baker). Wall and floor pieces are slabs ("1 m" is in-plane size).
- Break: piece health from material and size; at zero it splits into children and severs connections inside the damage volume; groups cut from anchors detach as rigid groups and can be broken again; destroyed 1 m pieces become local VFX. Large hits cascade several levels in one event. Melee uses about a 0.5 m radius.
- Integrity: connectivity graph with strength per connection, foundation pieces as anchors; v1 is connectivity plus strain, sag is a stretch goal on the same graph.
- Materials are data assets (concrete baseline, wood weaker, metal stronger). Glass is a separate system (one replicated bit per pane, any damage shatters it).
- Props are separate actors with health and per-prop strength; destroyed state is one replicated flag plus local particles; carriable; valuables are props with an objective tag.
- Debris persists as non-simulating rubble, synchronized in groups (see rubble rule above).
- Players: debris deals momentum damage; overlapping debris lifts the player to the nearest free spot on top plus damage; no trapping. Auto-step about 0.5 m, vault up to about 1.2 m (tunable). Falling groups damage other structures.
- Network: host-authoritative listen server for 4 players; direct IP or LAN in development, Steam sessions later through the engine's Steam plugins; no host migration; legacy replication (Iris is production-supported in 5.8 and can be revisited, structs stay Iris-friendly).
- Vehicles: the driver's machine simulates the vehicle and the host validates damage; riders attach to seat slots; build order crane, bulldozer, truck.
- Baker: one click, validates geometry (watertight check), classifies shell/prop/glass, assigns materials (Physical Material first, then keyword table, fallback concrete with a warning, per-mesh override tag), builds fracture tree and connectivity graph with anchors.
- Demolition mode: progress is the share of objective-structure volume no longer connected to its anchors; fail on valuable destroyed or timer end; rounds reset by reloading the map; `GameMode`/`GameState` C++ bases, Blueprint subclasses, rules data asset; no Game Feature plugins.
- Project: new C++ First Person template project on the Launcher 5.8.3, no engine changes; own replicated character on CharacterMovement; Enhanced Input kept; no Gameplay Ability System or Common UI; dynamic lighting with Lumen off by default, Nanite decided after the spike.
- Process: Git with LFS and feature branches; approval rules in section 10; docs as Markdown in `/Docs`.

## Appendix B: Defaults assumed (questions not yet answered)

Change any of these by editing `DECISIONS.md`.

- Max depth 4 with a 16 m root cap.
- A player at 0 HP respawns at a spawn point after a short delay.
- Valuables only need to survive; no delivery zone.
- Only props tagged carriable can be carried.

## Appendix C: Session starter prompt

> Read `Docs/PROJECT_STATE.md`, `Docs/DECISIONS.md` and `Docs/DEV_WORKFLOW_MCP.md`. Confirm the editor is reachable with `list_toolsets`. Then tell me the current milestone, the next task, the plan for this session in a few lines, and the test that will prove it. Do not change anything until I say go.
