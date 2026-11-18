#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructionTestActor.generated.h"

class UGeometryCollectionComponent;
class UFieldSystemComponent;

UCLASS()
class DEMOCO_API ADestructionTestActor : public AActor
{
	GENERATED_BODY()

public:
	ADestructionTestActor();

	// Triggered via key event to test spatial destruction
	UFUNCTION(BlueprintCallable, Category = "Destruction")
	void RunDestructionTest();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction")
	TObjectPtr<UFieldSystemComponent> FieldSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction")
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;

	// Essential parameters only
	UPROPERTY(EditAnywhere, Category = "Destruction|Test")
	float TestStrain = 500000.0f;

	UPROPERTY(EditAnywhere, Category = "Destruction|Test")
	float TestRadius = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Destruction|Test")
	FVector LocalImpactOffset = FVector(0.0f, 0.0f, 100.0f);

	void ApplyTestStrain(FVector ImpactPoint, float Strain, float Radius);
};