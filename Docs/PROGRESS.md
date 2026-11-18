# Progress log

## 2026-11-18 - Fracture pattern revised: irregular cells, aim offset accepted
**What changed:** docs only. `DECISIONS.md`, `ARCHITECTURE.md`, `DEV_WORKFLOW_MCP.md` (version 3, also fixed a stale section reference), `NEXT_STEPS.md`, `PROJECT_STATE.md`. No source, config or asset changes.

**Why:** you said some offset between the aim point and the broken piece is fine, like The Finals. That removes the main reason the strict 1 m lattice was chosen. The design now uses irregular baked cells with leaves of about 1 m, and the baker stores which cells touch. This drops the power-of-two partition and the remainder rule. The lattice stays as the fallback if Spike B cannot bake the new pattern from editor code. Replication, hash and group rules are unchanged because they use node IDs and cut edges.

**New Unreal concepts:**
- *Voronoi fracture:* split a mesh into cells around seed points, so each cell is irregular. Unreal's Fracture mode and Geometry Script can do this.
- *Adjacency:* a list of which pieces touch which. The solver builds the connectivity graph from it, instead of assuming a grid.

## 2026-11-18 - Design review: replication model, fracture pattern, milestone changes
**What changed:** docs only. Rewrote `DECISIONS.md` and `ARCHITECTURE.md`; updated `DEV_WORKFLOW_MCP.md` (version 2: modules, testing, deferred toolset, approval note, milestones, appendix), `PROJECT_STATE.md`, `NEXT_STEPS.md`, `SETUP_NOTES.md`. No source, config or asset changes.

**Why:** a review of the docs against your answers and the repo found gaps. Replicated state needed cut-edge bits, because groups form when connections are cut even if no piece dies. Detached groups needed a replication rule (one transform per group, only while awake, with settle and deadband rules). Intact buildings needed a rendering plan, since about 100k pieces cannot be components. The fracture pattern needed a lattice rule so leaves are exactly 1 m. The repo also had a prototype vault that the docs did not mention, and rendering config that contradicts the 60 fps target.

**New Unreal concepts:**
- *Dormancy:* an actor that is not changing can be marked dormant so replication skips it. Structures use it between break events.
- *Deadband and quantization:* send a transform only if it changed enough, and store it with fewer bits. Both cut bandwidth for objects that jitter.
- *Client prediction in CharacterMovement:* the client moves first and the server confirms. Server-only `LaunchCharacter` skips the prediction, so remote clients get corrected (rubber-banding). Root motion sources and custom movement modes are the prediction-friendly options.
- *Instanced static mesh:* many copies of one mesh drawn as one component. Individual instances can be hidden, which is the candidate way to show a building that breaks into pieces.

## 2026-11-18 - M0 session 4: Live Coding compile passes, cleanup planned
**What changed:** on `feature/m0-setup`, changed one log string in `DemoCoGameMode.cpp`, compiled it through `LiveCodingToolset.CompileLiveCoding` (`Success`), reverted the string and compiled again (`Success`). Source is unchanged at the end. Updated `SETUP_NOTES.md` (item 3 now PASS, new quirks), `PROJECT_STATE.md` and `NEXT_STEPS.md`. Checked what depends on the old `Destruction_structure` script and wrote a cleanup plan; nothing was deleted yet.

**Why:** Day-0 item 3 needed a real compile, not `NoChanges`. The first two attempts returned `NoChanges` because `DemoCo.dll` had not been loaded by the editor, so Live Coding ignored the edit. A Visual Studio build fixed it. The old script cannot be removed in one step: a Blueprint derives from it and a level actor uses that Blueprint.

**New Unreal concepts:**
- *Live Coding patch:* a changed function body is compiled into a small patch DLL (`UnrealEditor-DemoCo.patch_N`) that is loaded into the running editor. It only works for a module whose normal DLL the editor already loaded.
- *External actors (One File Per Actor):* each actor in a level is saved as its own file under `Content/__ExternalActors__/`, so a placed actor shows up as an asset referencer of its Blueprint.
- *Blueprint parent class:* a Blueprint inherits from a C++ class; removing the C++ class leaves the Blueprint broken, so Blueprint and level instances go first.

## 2026-11-18 - M0 session 3: Live Coding toolset, log reading, PIE
**What changed:** confirmed `LiveCodingToolset` loads and `CompileLiveCoding` returns `NoChanges` on a clean build; read the Output Log through `LogsToolset`; started and stopped PIE (listen server plus 1 client) through `EditorAppToolset`; updated `SETUP_NOTES.md`, `PROJECT_STATE.md`, `NEXT_STEPS.md`. No source, config or asset changes.

**Why:** Day-0 items 3 and 6 needed real calls. The trivial-change compile is deferred because the repo is on `main` and the approval rules only allow source edits on a feature branch.

**New Unreal concepts:**
- *Listen server PIE:* one editor process hosts the match and also runs a local client; extra clients connect to it over loopback (port 17777 in the log).
- *Log categories:* `LogLiveCoding`, `LogNet` and `PIE` let you filter the Output Log to one subsystem.

## 2026-11-18 - M0 session 2: file access and Live Coding check
**What changed:** verified filesystem connector access (Day-0 item 5 passes); found why `LiveCodingToolset` is missing; added `NEXT_STEPS.md` (plan plus Claude Desktop prompt); updated `PROJECT_STATE.md`, `SETUP_NOTES.md`, `README.md`. No C++, config or plugin changes.

**Why:** enabling Live Coding in the editor was not enough. MCP exposes `LiveCodingToolset` only when the plugin of the same name is enabled, and it is disabled. Enabling plugins needs your approval (section 10) and an editor restart.

**New Unreal concepts:**
- *Plugin vs editor setting:* Live Coding the feature is an editor preference; the MCP tool wrapper is a separate plugin.
- *Plugin enable timing:* `SetPluginEnabled` edits project config and takes effect on the next editor restart.

## 2026-11-18 - M0 session 1: Day-0 checks and docs skeleton
**What changed:** ran Day-0 items 1, 2 and 4 through MCP; recorded results in `SETUP_NOTES.md`; created `README.md`, `DECISIONS.md`, `ARCHITECTURE.md`, `SETUP_NOTES.md`, `PROGRESS.md`; updated `PROJECT_STATE.md`. No C++ or assets were touched.

**Why:** M0 is done only when the workflow's assumptions are verified. Item 3 (Live Coding via MCP) fell back to manual compile because the toolset is not exposed.

**New Unreal concepts:**
- *Automation tests:* engine test framework. `DiscoverTests` loads the list, then tests run by full path (for example `System.Core.Math.FColor.Smoke Test`).
- *Toolsets:* groups of AI-callable functions registered with `ToolsetRegistry`; reached through `list_toolsets`, `describe_toolset`, `call_tool`.
- *Live Coding:* recompiles C++ function bodies while the editor runs. Class/property changes still need a full rebuild.
