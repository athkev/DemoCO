# Decisions

Approved decisions plus a dated log. Edit here to change a default.

## Answered questions (Appendix B defaults, all confirmed "yes")
- Max depth 4 with a 16 m root cap.
- A player at 0 HP respawns at a spawn point after a short delay.
- Valuables only need to survive; no delivery zone.
- Only props tagged carriable can be carried.

## Approved decisions (from Appendix A of `DEV_WORKFLOW_MCP.md`)
- **Fracture tree:** root to 1 m leaves, size halving per level, max depth 4. Root piece cap about 16 m; larger structures are split into several root pieces by the baker. Wall and floor pieces are slabs ("1 m" is in-plane size).
- **Break:** piece health from material and size. At zero it splits into children and severs connections inside the damage volume. Groups cut from anchors detach as rigid groups and can be broken again. Destroyed 1 m pieces become local VFX. Large hits cascade several levels in one event. Melee radius about 0.5 m.
- **Integrity:** connectivity graph with strength per connection, foundation pieces as anchors. v1 is connectivity plus strain; sag is a stretch goal on the same graph.
- **Materials:** data assets (concrete baseline, wood weaker, metal stronger). Glass is a separate system (one replicated bit per pane, any damage shatters it).
- **Props:** separate actors with health and per-prop strength. Destroyed state is one replicated flag plus local particles. Carriable. Valuables are props with an objective tag.
- **Debris:** persists as non-simulating rubble, synchronized in groups. Old rubble is not removed. A live-piece cap is added only if M5 or M9 profiling shows group sync is not enough (last resort).
- **Players:** debris deals momentum damage. Overlapping debris lifts the player to the nearest free spot on top plus damage; no trapping. Auto-step about 0.5 m, vault up to about 1.2 m (tunable). Falling groups damage other structures.
- **Network:** host-authoritative listen server for 4 players. Direct IP or LAN in development, Steam sessions later via engine Steam plugins. No host migration. Legacy replication (Iris can be revisited; keep structs Iris-friendly).
- **Vehicles:** driver's machine simulates, host validates damage. Riders attach to seat slots. Build order: crane, bulldozer, truck.
- **Baker:** one click. Validates geometry (watertight), classifies shell/prop/glass, assigns materials (Physical Material, then keyword table, fallback concrete with warning, per-mesh override tag), builds fracture tree and connectivity graph with anchors.
- **Demolition mode:** progress = share of objective-structure volume no longer connected to anchors. Fail on valuable destroyed or timer end. Rounds reset by reloading the map. C++ `GameMode`/`GameState` bases, Blueprint subclasses, rules data asset. No Game Feature plugins.
- **Project:** new C++ First Person template on Launcher 5.8.3, no engine changes. Own replicated character on CharacterMovement. Enhanced Input kept. No GAS or Common UI. Dynamic lighting, Lumen off by default, Nanite decided after the spike.
- **Process:** Git with LFS and feature branches; approval rules in section 10 of the workflow doc; docs as Markdown in `/Docs`.

## Change log
- 2026-11-18: Initial decisions recorded. Appendix B defaults confirmed.
