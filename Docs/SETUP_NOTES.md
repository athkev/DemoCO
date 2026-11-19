# Setup notes

Project path: `E:\unity\projects\DemoCo`
Engine: UE 5.8.3 (Launcher build, `E:\UnrealEngineEditor\UE_5.8`)
MCP server: `http://127.0.0.1:8000/mcp`
Startup level seen via MCP: `/Game/FirstPerson/Lvl_FirstPerson`
Project module: `DemoCo` (Runtime). Plugins enabled in `DemoCo.uproject` include `ModelContextProtocol`, `AllToolsets`, `MCPClientToolset`, `VisualStudioTools`.

## Day-0 results (2026-11-18)
| # | Check | Result | Notes |
|---|---|---|---|
| 1 | Server reachable | PASS | `list_toolsets` returned the catalog (51 toolsets in this client's list after `RefreshTools`). |
| 2 | Read-only call | PASS | `SceneTools.get_current_level` -> `/Game/FirstPerson/Lvl_FirstPerson`. |
| 3 | C++ compile via MCP | PASS | `LiveCodingToolset` loads (plugin enabled by user). Baseline `CompileLiveCoding` returned `NoChanges`. On `feature/m0-setup`, a one-line log-string change in `DemoCoGameMode.cpp` first returned `NoChanges` too, because `DemoCo.dll` had not been loaded by the editor (Live Coding console: "has not been loaded by any process. Changes will be ignored"). After the user built Development Editor from Visual Studio, the next edit compiled: `Result: Success`, patch `UnrealEditor-DemoCo.patch_0`; a revert compiled as `patch_1`. Test edit was reverted; source is unchanged. |
| 4 | Automation tests via MCP | PASS | `DiscoverTests` ready; `ListTests` filter `System.Core.Math` found 40; `RunTests` on 2 tests -> 2 passed, 0 failed. No project tests yet (expected). |
| 5 | Source file access | PASS | Filesystem connector allows `E:\unity\projects\DemoCo` (read, write, list, edit). `AssetTools.read_file/write_file` stay limited to `Content/` and `Saved/`. |
| 6 | Multi-client PIE / log | PASS (2 players) | `LogsToolset.GetLogEntries` reads the Output Log (also filtered by category). `EditorAppToolset.StartPIE` launched listen server plus 1 client from current editor play settings (log: server listening on 17777, client joined, `BP_ShooterGameMode`). `StopPIE` ended it. `StartPIE` has no player-count or net-mode option, so 3 to 4 players needs the editor Play settings changed (ask first, config change). |
| 7 | Git | FALLBACK (by design) | `.git` exists. User runs Git; Claude writes commands. |
| 8 | Record setup | PASS | See below. |

## MCP client entries
- Repo `.mcp.json` (for Claude Code and similar):
  ```json
  { "mcpServers": { "unreal-mcp": { "type": "http", "url": "http://127.0.0.1:8000/mcp" } } }
  ```
- Claude Desktop: connected and working (user confirmed). Exact entry (direct URL vs `unreal_mcp_proxy.exe`) not yet pasted here. TODO: paste it.
- Filesystem connector: scoped to `E:\unity\projects\DemoCo`.
- The repo `.mcp.json` is for Claude Code and is unused by Claude Desktop. The Desktop entry for a local HTTP server is most likely the STDIO proxy (`unreal_mcp_proxy.exe`); paste the working entry here when you have it.

## Git
- Remote: GitHub (`origin`). LFS installed; `.gitattributes` tracks `.uasset`, `.umap`, `.fbx`, `.obj`, `.png`, `.psd`, `.tga`, `.wav`, `.mp4`.
- Open: check `git lfs ls-files` for binaries committed before LFS (they stay in normal history until migrated); consider more extensions in `.gitattributes`; consider `.idea/`, `PackageBuild/`, `*.user` in `.gitignore`. See `NEXT_STEPS.md`.

## Baseline project config (template defaults, to be reviewed in Spike C)
`Config/DefaultEngine.ini` currently has Lumen GI and reflections (`r.DynamicGlobalIlluminationMethod=1`, `r.ReflectionMethod=1`), hardware ray tracing on (`r.RayTracing=True`), Substrate on, virtual shadow maps on, DX12 with SM6. The design target is 60 fps on mid-range hardware with dynamic lighting and Lumen off by default. Changing these needs your approval (section 10 of the workflow doc) and a baseline measurement first.

## MCP permissions
All calls go through the single `call_tool`, so Claude Desktop's permission prompt cannot separate reads from deletes. Keep per-call approval on for `call_tool` and commit before sessions that edit assets.

## Quirks
- `call_tool` takes the bare tool name (`get_current_level`), not the prefixed name from `describe_toolset`.
- `AssetTools` file access is limited to `Content/`, `Saved/` and engine plugin content.
- `LiveCodingToolset` is its own plugin: enable it, restart the editor, then `RefreshTools`. `SetPluginEnabled` alone did not take effect until the user enabled it in the editor.
- Template game mode seen in PIE: `/Game/Variant_Shooter/Blueprints/BP_ShooterGameMode`.
- A client may keep a stale toolset list after `RefreshTools`; verify with a read-only call.
- `CompileLiveCoding` returning `NoChanges` after a real edit usually means the module DLL is not loaded by the editor (no build exists for it yet, or the editor was started before the build). Fix that worked here: build `Development Editor` for Windows from Visual Studio, then edit again. (Unknown whether the editor had to be closed or restarted; a build with the editor open normally needs a restart or Live Coding.) Check `LogsToolset.GetLogEntries` with category `LogLiveCoding`; the console's own message is only visible in its window.
- If Visual Studio's build already contains an edit, Live Coding sees no difference. Make a new change to test a compile.
- A first Live Coding compile after a fresh build can take about 18 s; patches after that took about 2.5 s.
