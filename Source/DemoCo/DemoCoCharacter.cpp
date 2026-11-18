// Copyright Epic Games, Inc. All Rights Reserved.

#include "DemoCoCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DemoCo.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADemoCoCharacter::ADemoCoCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void ADemoCoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADemoCoCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADemoCoCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADemoCoCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADemoCoCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADemoCoCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogDemoCo, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ADemoCoCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ADemoCoCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ADemoCoCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ADemoCoCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ADemoCoCharacter::DoJumpStart()
{
	bJumpHeld = true;
	bVaultedThisJump = false;
	// pass Jump to the character
	Jump();
}

void ADemoCoCharacter::DoJumpEnd()
{
	bJumpHeld = false;
	// pass StopJumping to the character
	StopJumping();
}

void ADemoCoCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Poll VaultCheck while Jump is held (local client only)
	if (bJumpHeld && !bVaultedThisJump && IsLocallyControlled())
	{
		VaultCheck();
	}
}

void ADemoCoCharacter::VaultCheck()
{
	UWorld* World = GetWorld();
	if (!World) return;

	const FVector ActorLocation = GetActorLocation();
	const FVector ForwardDir    = GetActorForwardVector();
	const float   HalfHeight    = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// Setup Object Query Params to ONLY hit WorldStatic and WorldDynamic (ignoring Pawns/Characters)
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// Trace 1 – Waist-height forward trace (detects obstacle)
	const FVector WaistOrigin = ActorLocation + FVector(0.f, 0.f, -HalfHeight * 0.5f);
	const FVector Trace1End   = WaistOrigin + ForwardDir * VaultTraceForwardDistance;

	FHitResult WaistHit;
	bool bHitObstacle = World->LineTraceSingleByObjectType(
		WaistHit,
		WaistOrigin,
		Trace1End,
		ObjectQueryParams,
		QueryParams
	);

	if (!bHitObstacle) return; // nothing to vault over

	// Trace 2 – Downward trace from head + 80 cm forward (finds ledge top)
	const float   HeadZ          = ActorLocation.Z + HalfHeight;
	const FVector LedgeProbe     = ActorLocation + ForwardDir * (VaultTraceForwardDistance + VaultTraceDownwardStart);
	const FVector Trace2Start    = FVector(LedgeProbe.X, LedgeProbe.Y, HeadZ + VaultTraceDownwardStart);
	const FVector Trace2End      = FVector(LedgeProbe.X, LedgeProbe.Y, ActorLocation.Z - HalfHeight); // sweep to feet level

	FHitResult LedgeHit;
	bool bHitLedge = World->LineTraceSingleByObjectType(
		LedgeHit,
		Trace2Start,
		Trace2End,
		ObjectQueryParams,
		QueryParams
	);

	if (!bHitLedge) return; // no ledge surface found

	// Height constraint:  50 cm <= ledge height above feet <= 200 cm
	const float FeetZ       = ActorLocation.Z - HalfHeight;
	const float LedgeHeight = LedgeHit.ImpactPoint.Z - FeetZ;

	if (LedgeHeight < VaultMinHeight || LedgeHeight > VaultMaxHeight) return;

	// Valid vault – tell the server
	const FVector LedgeLocation = LedgeHit.ImpactPoint;
	Server_VaultTo(LedgeLocation);

	bVaultedThisJump = true; // suppress repeat until next key press
}

bool ADemoCoCharacter::Server_VaultTo_Validate(FVector LedgeLocation)
{
	return FVector::Dist(LedgeLocation, GetActorLocation()) <= 500.f;
}

void ADemoCoCharacter::Server_VaultTo_Implementation(FVector LedgeLocation)
{
	const float FeetZ       = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float LedgeHeight = LedgeLocation.Z - FeetZ;

	if (LedgeHeight < VaultMinHeight || LedgeHeight > VaultMaxHeight) return;

	const FVector ToLedge    = (LedgeLocation - GetActorLocation()).GetSafeNormal();
	const FVector LaunchVel  = FVector(ToLedge.X * 400.f, ToLedge.Y * 400.f, VaultLaunchZSpeed);

	LaunchCharacter(LaunchVel, true, true);
}
