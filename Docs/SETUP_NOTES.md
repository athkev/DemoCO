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
| 3 | C++ compile via MCP | PARTIAL | `LiveCodingToolset` now loads (plugin enabled by user, 53 toolsets). `CompileLiveCoding` ran: `Result: NoChanges`, "Live coding succeeded, no code changes detected" (LogLiveCoding confirms). Still to do: a compile with a trivial function-body change in `Source/DemoCo`. Not done yet because the repo is on `main`; source edits wait for branch `feature/m0-setup`. |
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

## Quirks
- `call_tool` takes the bare tool name (`get_current_level`), not the prefixed name from `describe_toolset`.
- `AssetTools` file access is limited to `Content/`, `Saved/` and engine plugin content.
- `LiveCodingToolset` is its own plugin: enable it, restart the editor, then `RefreshTools`. `SetPluginEnabled` alone did not take effect until the user enabled it in the editor.
- Template game mode seen in PIE: `/Game/Variant_Shooter/Blueprints/BP_ShooterGameMode`.
- A client may keep a stale toolset list after `RefreshTools`; verify with a read-only call.
