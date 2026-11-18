# Architecture

Status: skeleton (M0). Sections are filled in as milestones land. Source of intent: `DECISIONS.md`.

## Modules (planned)
| Module | Contents | Notes |
|---|---|---|
| `DestructionSolver` | Fracture tree, connectivity graph, strain solver | Pure C++, no UObject, headless unit tests |
| `DestructionCore` | Runtime types, destructible asset, structure actor, event pipeline, replication | |
| `DestructionGameplay` | Tools, props, glass, debris, vehicles, Demolition mode | |
| `DestructionEditor` | Baker, validation, project MCP toolset | Editor-only |

Content lives under `Content/Destruction/{Materials,Buildings,Props,Tools,Vehicles,Modes}/`.
Conventions: Unreal prefixes (`SM_`, `BP_`, `DA_`, `M_`, `MI_`, `NS_`), one class per file, logic in C++, Blueprints only for thin variants, tuning in data assets.

## Data flow (intent)
Damage source (`FDestructionImpulse`) -> host solver -> break event -> replicated alive-piece bitset -> clients apply and spawn local VFX. Late join receives a snapshot.

## Replication design
Host-authoritative listen server, legacy replication. Alive-piece state is a replicated bitset per structure. Settled rubble syncs in groups. The runtime approach (Chaos Geometry Collection vs custom chunk events) is decided in M1 Spike A.

## Open (decided in M1)
- Runtime approach: Chaos GC replication vs custom chunk events.
- Baker approach: fracture/Dataflow or Geometry Script driven from editor code.
- Performance budgets.
