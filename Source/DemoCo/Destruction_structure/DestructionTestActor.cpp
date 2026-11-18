#include "DestructionTestActor.h"
#include "Components/SceneComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemNodes.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

ADestructionTestActor::ADestructionTestActor()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
    FieldSystem->SetupAttachment(SceneRoot);
}

void ADestructionTestActor::BeginPlay()
{
    Super::BeginPlay();

    GeometryCollection = FindComponentByClass<UGeometryCollectionComponent>();
    if (!GeometryCollection || !FieldSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("%s: Missing GeometryCollection or FieldSystem component!"), *GetName());
    }
}

void ADestructionTestActor::RunDestructionTest()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!GeometryCollection || !FieldSystem)
    {
        return;
    }

    const FVector ImpactPoint = GeometryCollection->GetComponentTransform().TransformPosition(LocalImpactOffset);

    // Draw debug sphere matching the exact test radius
    DrawDebugSphere(
        GetWorld(),
        ImpactPoint,
        TestRadius,
        16,
        FColor::Red,
        false,
        5.0f,
        0,
        3.0f
    );

    ApplyTestStrain(ImpactPoint, TestStrain, TestRadius);

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Destruction Executed | Point: %s | Radius: %.1f"),
        *ImpactPoint.ToString(),
        TestRadius
    );
}
void ADestructionTestActor::ApplyTestStrain(FVector ImpactPoint, float Strain, float Radius)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!GeometryCollection)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyTestStrain: GeometryCollection component is NULL!"));
        return;
    }

    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;

    // Use a zero-length sphere sweep (from ImpactPoint to ImpactPoint) 
    // to collect individual hit items/bones via FHitResult without positional offset drift.
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        ImpactPoint,
        ImpactPoint, // Zero-length vector keeps the sphere centered precisely on ImpactPoint
        FQuat::Identity,
        ECC_WorldStatic, // Match your Geometry Collection's Object Type
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    int32 FracturedCount = 0;
    if (bHit)
    {
        TSet<int32> ProcessedItems;
        for (const FHitResult& Hit : HitResults)
        {
            if (Hit.Component.Get() == GeometryCollection)
            {
                int32 ItemIndex = Hit.Item; // FHitResult safely provides the exact bone/piece index
                
                if (!ProcessedItems.Contains(ItemIndex))
                {
                    ProcessedItems.Add(ItemIndex);

                    GeometryCollection->ApplyExternalStrain(
                        ItemIndex,
                        ImpactPoint,
                        Radius,
                        1,    // Propagation depth
                        1.0f, // Propagation factor
                        Strain
                    );

                    FracturedCount++;
                }
            }
        }
    }

    UE_LOG(
        LogTemp,
        Display,
        TEXT("Radial Destruction Executed | Target Pieces Found: %d | Point: %s | Radius: %.1f"),
        FracturedCount,
        *ImpactPoint.ToString(),
        Radius
    );
}