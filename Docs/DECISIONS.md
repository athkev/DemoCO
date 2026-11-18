# Decisions

Approved decisions plus a dated log. Edit here to change a default. This file is authoritative; the summary in Appendix A of `DEV_WORKFLOW_MCP.md` may lag.

## Answered questions (Appendix B defaults, all confirmed "yes")
- Max depth 4 with a 16 m root cap.
- A player at 0 HP respawns at a spawn point after a short delay.
- Valuables only need to survive; no delivery zone.
- Only props tagged carriable can be carried.

## Answered in the 2026-11-18 design review
- Replication: attached pieces are processed and replicated as one unit until separated; only moving or falling groups replicate; small shaking is filtered (see "Replication and sync").
- Fracture pattern: Finals-like. Smallest piece is about 1 m wide. Revised later in the day: some offset between aim point and broken piece is acceptable, so leaves need not be exactly 1 m (see "Fracture and break").
- Sag: wanted if the design supports it. It does (same graph), so it is planned after M5.
- Vehicle riders: seat slots for riders who enter; a player standing on top of a vehicle without a seat still moves with it.
- Fracture pattern details left to engineering: decided below.

## Approved decisions

### Fracture and break
- **Fracture tree:** root to leaves of about 1 m, max depth 4. Root piece cap about 16 m; larger structures are split into several root pieces by the baker. Wall and floor pieces are slabs ("1 m" is in-plane size).
- **Pattern (revised 2026-11-18, supersedes the strict lattice):** irregular, Voronoi-style cells at every level, baked into a hierarchy. Leaves have a nominal size of about 1 m with a tolerated range (starting guess 0.7 to 1.4 m, tuned in Spike B). No lattice formula, no power-of-two partition, no remainder rule: the baker fits cells to any wall length. The baker stores each node's bounds and which cells touch (adjacency), so the solver never assumes a grid.
- **Accuracy tolerance (new):** some offset between the aim point and the broken piece is accepted, as in The Finals. Starting cap: the centre of a broken leaf is within about 0.5 m of the hit point (tuned by feel in Spike B and M4).
- **Fallback:** if Spike B cannot produce a clean cell hierarchy from editor code, use the previous design: a 1 m lattice, binary partition (1, 2, 4, 8, 16 m), greedy remainders (10 m = 8 + 2), jagged cuts with displacement about 0.15 m. Spike B compares both looks.
- **Hit test and routing:** a hit is a sphere (radius, damage) from the tool and material tables. It damages every alive node whose bounds it overlaps at the depth whose average cell size is the largest that is at most twice the hit radius (never finer than the leaf level). A hit near a boundary or corner overlaps all neighbours, so a sledgehammer (about 0.5 m radius) there breaks several cells. Large hits cascade several levels in one event. Health is created lazily on first damage.
- **Break:** piece health from material and size. At zero the piece splits into children and severs connections inside the damage volume. Groups cut from anchors detach as rigid groups and can be broken again. Destroyed 1 m pieces become local VFX.
- **Materials:** data assets (concrete baseline, wood weaker, metal stronger; each also holds sound and VFX references). Small hits take many hits and big hits one, through the hit tables above. Glass is a separate system (one replicated bit per pane, any damage shatters it).

### Integrity
- Connectivity graph with strength per connection, foundation pieces as anchors. Nodes carry mass and edges carry capacity from M2, so strain now and sag later use the same graph. v1 is connectivity plus strain. Sag is planned after M5 (M5b) and cut only if it threatens the frame budget.

### Replication and sync (new, 2026-11-18)
- **Clients do not run the solver.** The host solves; clients apply results. This avoids divergence.
- **State model:** per structure, per tree node a state (intact, split, destroyed), plus one bit per connection (cut or not). Group membership is decided by the host and sent as events. The sync hash covers node state and cut-edge bits.
- **Attached = one unit.** While pieces are connected to an anchor they are static, cost nothing per frame, and the structure replicates only on break events (dormant otherwise). Once a set of connected pieces is separated it becomes one rigid group: one body and one replicated transform per group, not per piece. A group that breaks further splits into child groups that inherit velocity.
- **Transport:** sequenced break events for state, a snapshot for late join, and separate unreliable cosmetic events (impact point, radius, impulse) for VFX and sound.
- **Only moving groups replicate.** Awake groups replicate quantized transforms at a capped rate scaled by distance; clients interpolate and never simulate.
- **Shaking and settling:** (1) a group sleeps when linear and angular speed stay under a threshold for a settle time (hysteresis), then the host sends one reliable final transform and stops; clients snap and freeze. (2) Deadband: no update if position or rotation changed less than a small threshold since the last send. (3) Micro-contacts and sub-threshold impacts do not wake settled rubble; only a break event or an impact above an impulse threshold does. (4) A maximum awake time forces sleep. (5) A cap on awake groups. All values live in a data asset. Starting guesses, to be tuned in Spike C and M5: sleep under 5 cm/s for 1 s, deadband 1 cm and 0.5 degrees, quantization 1 cm, 10 to 20 Hz.
- **Rubble:** settled rubble persists as non-simulating groups, still damageable by the same solver. If rubble cost becomes a problem, merging settled groups is the first fallback; a live-piece cap stays the last resort.

### Rendering (new)
- Intact structures cannot be one component or actor per piece (about 100k pieces for 30 buildings). Candidate: instanced meshes per material per structure with per-instance hiding, plus a small proxy actor per detached group. Decided in Spike C.
- Lighting: the template config still has Lumen, hardware ray tracing, Substrate and virtual shadow maps. Spike C records a baseline, then config changes are proposed to you (config changes need approval). Goal: dynamic lighting without Lumen by default unless the baseline says otherwise.

### Props, players, vehicles
- **Props:** separate actors with health and per-prop strength. Destroyed state is one replicated flag plus local particles. Carriable. Valuables are props with an objective tag.
- **Players:** debris deals momentum damage. Overlapping debris lifts the player to the nearest free spot on top plus damage; no trapping. Auto-step about 0.5 m, vault up to about 1.2 m (tunable). Falling groups damage other structures. The existing vault code in `DemoCoCharacter` is a prototype only and is redone in M3 so it works with client prediction.
- **Vehicles:** driver's machine simulates, host validates damage. Riders who enter attach to seat slots. A player standing on top of a vehicle without a seat moves with it through a movement base. This is a known risk, verified in M8; the fallback is to auto-attach a player who stays on top. Build order: crane, bulldozer, truck.

### Network and project
- **Network:** host-authoritative listen server for 4 players. Direct IP or LAN in development; a Steam session test with the test App ID 480 over the internet by M4, not M9. No host migration. Legacy replication (Iris can be revisited; keep structs Iris-friendly).
- **Baker:** one click. Validates geometry (watertight), classifies shell/prop/glass, assigns materials (Physical Material, then keyword table, fallback concrete with warning, per-mesh override tag), builds the fracture tree and connectivity graph with anchors. A thin version runs in M2 on one real hollow lowpoly building so the solver is not built only against generated boxes.
- **Demolition mode:** progress = share of objective-structure volume no longer connected to anchors. Fail on valuable destroyed or timer end. Rounds reset by reloading the map. C++ `GameMode`/`GameState` bases, Blueprint subclasses, rules data asset. No Game Feature plugins.
- **Project:** new C++ First Person template on Launcher 5.8.3, no engine changes. Own replicated character on CharacterMovement. Enhanced Input kept. No GAS or Common UI.
- **Modules (changed):** keep the single `DemoCo` runtime module with folders `Destruction/Solver` (no UObject, namespace `Destruction`, headless tests), `Destruction/Core` and `Destruction/Gameplay`. Add an editor-only `DemoCoEditor` module when the baker starts. A separate solver module is split out later only if needed.
- **Project toolset (changed):** the custom `DestructionToolset` is deferred; until M6, repeatable scenarios use console commands plus Automation tests.
- **Spikes (changed):** Spike A uses custom state and replication as the baseline and compares Chaos Geometry Collection for one day; Chaos is only considered for simulating detached groups. Spike C (rendering and cost at scale) is added.
- **Process:** Git with LFS and feature branches (LFS configured, see `SETUP_NOTES.md`); approval rules in section 10 of the workflow doc; docs as Markdown in `/Docs`; `CLAUDE.md` is read every session.

## Change log
- 2026-11-18: Initial decisions recorded. Appendix B defaults confirmed.
- 2026-11-18: Fracture pattern revised: irregular baked cells with nominal 1 m leaves and a tolerated aim offset replace the strict 1 m lattice; lattice kept as the Spike B fallback. Replication, hash and group rules unchanged (they use node IDs and cut edges).
- 2026-11-18: Design review applied. Added lattice and remainder rule, pattern, hit routing, replication state model, group-as-unit and settle rules, rendering intent, sag planned after M5, standing riders, module simplification, deferred toolset, Spike C, thin baker in M2, Steam test in M4. Prototype vault code recorded as a prototype.
