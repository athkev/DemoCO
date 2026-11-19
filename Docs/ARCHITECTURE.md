# Architecture

Status: design reviewed 2026-11-18, runtime not built yet. Sections get verified as milestones land. Source of intent: `DECISIONS.md`.

## Modules
| Where | Contents | Notes |
|---|---|---|
| `Source/DemoCo/Destruction/Solver` | Fracture tree, connectivity graph, strain solver | No UObject, namespace `Destruction`, headless Automation tests |
| `Source/DemoCo/Destruction/Core` | Runtime types, destructible asset, structure actor, event pipeline, replication | |
| `Source/DemoCo/Destruction/Gameplay` | Tools, props, glass, debris, vehicles, Demolition mode | |
| `Source/DemoCoEditor` | Baker, validation | Editor-only module, added when the baker starts |

One runtime module (`DemoCo`) keeps `Build.cs`, export macros and Live Coding simple for a first C++ project. The solver folder follows the no-UObject rule by convention and can become its own module later.

Content lives under `Content/Destruction/{Materials,Buildings,Props,Tools,Vehicles,Modes}/`.
Conventions: Unreal prefixes (`SM_`, `BP_`, `DA_`, `M_`, `MI_`, `NS_`), one class per file, logic in C++, Blueprints only for thin variants, tuning in data assets.

## Data flow
Damage source (`FDestructionImpulse`: point, radius, damage, impulse) -> host solver -> break events and group events -> clients apply them and spawn local VFX. Clients never run the solver. Late join receives a snapshot of node state, cut edges and awake groups.

## Fracture structure
- Each structure is split into root pieces of at most 16 m. Each root is a baked hierarchy of irregular (Voronoi-style) cells, max depth 4, down to leaves of about 1 m (tolerated range about 0.7 to 1.4 m). The baker fits cells to any wall length, so there is no lattice, power-of-two rule or remainder rule.
- Each node stores its bounds, parent, children and its touching neighbours (adjacency). The solver reads these and never assumes a grid.
- Walls and floors are slabs: "1 m" is the in-plane size.
- Aim offset is accepted (as in The Finals): the broken leaf's centre may be up to about 0.5 m from the hit point. Tuned by feel.
- A hit is a sphere. It damages alive nodes whose bounds it overlaps at the depth whose average cell size is the largest at most twice the hit radius, never finer than the leaf level. Boundary and corner hits overlap several cells, so all of them are damaged.
- Health is created lazily on first damage. A piece at zero health splits into children and severs connections inside the hit volume.
- Fallback if Spike B cannot bake this from editor code: 1 m lattice, binary partition (1, 2, 4, 8, 16 m), greedy remainders (10 m = 8 + 2), jagged cuts with small displacement.

## Connectivity and integrity
- Graph over the leaf cells, built from the baked adjacency. A cell inside an unbroken parent is welded to its siblings (infinite capacity), so only cut or weak connections matter.
- Nodes carry mass, edges carry capacity, foundation pieces are anchors. v1 solves connectivity plus strain once per event (batched, dirty region only), not once per piece.
- Sag (planned after M5) reuses the same fields: load accumulates through the graph, and under-supported nodes either sag or disconnect.

## Replication
Host-authoritative listen server, legacy replication.

**State** (per structure): node state (intact, split, destroyed) for every tree node, plus one bit per connection (cut or intact). Group membership is decided by the host and sent as events; it is not recomputed on clients.

**Transport:**
- Sequenced break events for state, applied in order, plus a snapshot for late join.
- Unreliable cosmetic events (impact point, radius, impulse) for VFX and sound. A state diff alone would lose the direction.
- Structures are separate actors and dormant while nothing changes.

**Groups as units:**
- Pieces still connected to an anchor are static and cost nothing per frame.
- A separated connected set becomes one rigid group: one body, one replicated transform. Breaking a group further splits it into child groups that inherit velocity.

**Movement replication:**
- Only awake groups replicate: quantized transform at a capped rate scaled by distance. Clients interpolate and never simulate.
- Settle: a group sleeps when speed stays under a threshold for a settle time (hysteresis). The host then sends one reliable final transform and stops; clients snap and freeze.
- Shaking filters: deadband on position and rotation; micro-contacts and weak impacts do not wake settled rubble; a maximum awake time forces sleep; a cap on awake groups.
- All thresholds live in a data asset. Starting values are in `DECISIONS.md` and are tuned in Spike C and M5.

**Settled rubble** persists as non-simulating groups and is damageable through the same solver. Merging settled groups is the first fallback if it costs too much.

**Checks:** `Destruction.DumpHash` prints a hash of node state plus cut edges; host and clients must match after every scenario.

## Rendering (decided in Spike C)
Intact structures cannot be one component per piece (about 100k pieces for 30 buildings). Candidate: instanced meshes per material per structure with per-instance hiding, and a small proxy actor per detached group. Nanite and lighting settings are decided after the spike. The template config currently has Lumen, hardware ray tracing, Substrate and virtual shadow maps on; changes are proposed to you after the baseline.

## Players
CharacterMovement with a custom, prediction-friendly vault (replaces the current prototype in `DemoCoCharacter`): the same movement is applied on client and server, either as a root motion source or as a custom movement mode. A server-only `LaunchCharacter` causes corrections on remote clients. Traces and collision must include the object type used by destruction pieces. A player lifted off overlapping debris is moved by the server and corrected on the client; this is rare and acceptable.

## Vehicles
Driver's machine simulates; the host validates damage. Riders who enter attach to seat slots. A player standing on a vehicle without a seat moves with it through a movement base (verified in M8; fallback is auto-attach after a short time on top).

## Open (decided in M1)
- Rendering representation and budgets (Spike C).
- Whether the irregular-cell hierarchy bakes cleanly and looks right, and whether Geometry Script or fracture/Dataflow produces it from editor code, versus the lattice fallback (Spike B).
- Custom state versus Chaos Geometry Collection for detached-group simulation (Spike A).
