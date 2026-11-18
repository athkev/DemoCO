# Progress log

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
