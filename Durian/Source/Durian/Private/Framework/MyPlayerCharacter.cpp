// Fill out your copyright notice in the Description page of Project Settings.


// ReSharper disable All
#include "Framework/MyPlayerCharacter.h"

#include "Abilities/AbilityEffectComponent.h"
#include "Abilities/MagnetTargetComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Constants/GameConstants.h"
#include "CollisionShape.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

// Sets default values
AMyPlayerCharacter::AMyPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	PhysicsHandle->LinearStiffness = 10000.0f;
	PhysicsHandle->LinearDamping = 1000.0f;
	PhysicsHandle->AngularStiffness = 5000.0f;
	PhysicsHandle->AngularDamping = 500.0f;
	
	AbilityEffect = CreateDefaultSubobject<UAbilityEffectComponent>(TEXT("AbilityEffect"));

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	bUseControllerRotationYaw = false;
}

void AMyPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Stencil 3 is reserved for the local player. Magnet world-scan materials
	// use it to keep the player out of the red structural treatment.
	TArray<UPrimitiveComponent*> PlayerPrimitiveComponents;
	GetComponents(PlayerPrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PlayerPrimitiveComponents)
	{
		if (IsValid(PrimitiveComponent))
		{
			PrimitiveComponent->SetRenderCustomDepth(true);
			PrimitiveComponent->SetCustomDepthStencilValue(3);
		}
	}

	SetActorTickEnabled(true);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AMyPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseMagnet();
	Super::EndPlay(EndPlayReason);
}

void AMyPlayerCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	switch (CurrentState)
	{
		case EPlayerState::Normal:
			{
				break;
			}
		case EPlayerState::MagnetTargeting:
			{
				UpdateMagnetTargeting();
				break;
			}
		case EPlayerState::MagnetControl:
			{
				UpdateMagnetControl();
				break;
			}
		case EPlayerState::Attack:
			{
				
				break;
			}
		default:
			{
				UE_LOG(LogTemp, Error, TEXT("It is not supported in EPlayerState: %s"), *UEnum::GetValueAsString(CurrentState));
				break;
			}
	}
}

void AMyPlayerCharacter::UpdateMagnetControl() const
{
	if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent() || !GetController())
	{
		return;
	}

	const FVector HoldLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	const FVector TargetLocation = HoldLocation + GetActorForwardVector() * MagnetDistance;
	FVector SafeTargetLocation = TargetLocation;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagnetTargetSweep), false, this);

	if (const AActor* GrabbedActor = PhysicsHandle->GetGrabbedComponent()->GetOwner())
	{
		QueryParams.AddIgnoredActor(GrabbedActor);
	}
	FHitResult SweepHit;
	
	// ReSharper disable once CppTooWideScope
	const bool bBlocked = GetWorld()->SweepSingleByChannel(SweepHit, HoldLocation, TargetLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(40.0f), QueryParams);

	if (bBlocked)
	{
		SafeTargetLocation = SweepHit.Location;
	}

	PhysicsHandle->SetTargetLocation(SafeTargetLocation);
}

void AMyPlayerCharacter::UpdateMagnetTargeting()
{
	const AController* PlayerController = GetController();
	
	if (!PlayerController)
	{
		ClearTargetedMagnetComponent();
		return;
	}

	FVector TraceStart;
	FRotator ViewRotation;
	
	PlayerController->GetPlayerViewPoint(TraceStart, ViewRotation);

	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * Constants::MagnetTargetRange;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	UPrimitiveComponent* HitComponent = bHit ? HitResult.GetComponent() : nullptr;
	const bool bIsValidMagnetTarget = HitResult.GetActor() && HitResult.GetActor()->FindComponentByClass<UMagnetTargetComponent>() && HitComponent && HitComponent->IsSimulatingPhysics();

	if (bIsValidMagnetTarget)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Green, FString::Printf(TEXT("Magnet Target: %s"), *GetNameSafe(HitResult.GetActor())));
		}

		SetTargetedMagnetComponent(HitComponent, HitResult.ImpactPoint);
		
		return;
	}

	ClearTargetedMagnetComponent();
}

void AMyPlayerCharacter::SetTargetedMagnetComponent(UPrimitiveComponent* NewTarget, const FVector& NewTargetLocation)
{
	if (TargetedMagnetComponent.IsValid() && TargetedMagnetComponent.Get() != NewTarget)
	{
		TargetedMagnetComponent->SetCustomDepthStencilValue(1);
	}

	TargetedMagnetComponent = NewTarget;
	TargetedMagnetLocation = NewTargetLocation;

	if (NewTarget)
	{
		NewTarget->SetRenderCustomDepth(true);
		NewTarget->SetCustomDepthStencilValue(2);
	}
}

void AMyPlayerCharacter::ClearTargetedMagnetComponent()
{
	if (TargetedMagnetComponent.IsValid())
	{
		TargetedMagnetComponent->SetCustomDepthStencilValue(1);
	}

	TargetedMagnetComponent.Reset();
	TargetedMagnetLocation = FVector::ZeroVector;
}

void AMyPlayerCharacter::HandleInteract()
{
	switch (CurrentState)
	{
	case EPlayerState::Normal:
		{
			break;
		}
	case EPlayerState::MagnetTargeting:
		{
			HandleMagnetSelect();
			break;
		}
	default:
		{
			UE_LOG(LogTemp, Error, TEXT("It is not supported in EPlayerState: %s"), *UEnum::GetValueAsString(CurrentState));
			break;
		}
	}
}

void AMyPlayerCharacter::HandleMagnetAction()
{
	if (CurrentState == EPlayerState::Normal)
	{
		SetPlayerState(EPlayerState::MagnetTargeting);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Cyan, TEXT("MAGNET MODE: ON"));
		}

		AbilityEffect->SetVisionEnabled(EAbilityType::Magnet, true);
		AbilityEffect->PlayEnterPulse(EAbilityType::Magnet);
		return;
	}

	// Pressing the ability-use input again while magnet mode is active cancels it.
	ReleaseMagnet();
}

void AMyPlayerCharacter::HandleMagnetSelect()
{
	if (CurrentState != EPlayerState::MagnetTargeting || !PhysicsHandle || !TargetedMagnetComponent.IsValid() || !GetController())
	{
		return;
	}

	UPrimitiveComponent* Component = TargetedMagnetComponent.Get();

	if (!Component->IsSimulatingPhysics())
	{
		ReleaseMagnet();
		
		return;
	}

	PhysicsHandle->GrabComponentAtLocation(Component, NAME_None, TargetedMagnetLocation);

	if (PhysicsHandle->GetGrabbedComponent())
	{
		const FVector HoldLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
		MagnetDistance = FMath::Clamp(FVector::DotProduct(TargetedMagnetLocation - HoldLocation, GetActorForwardVector()), MagnetMinDistance, MagnetMaxDistance);
		CurrentState = EPlayerState::MagnetControl;
	}
}

void AMyPlayerCharacter::HandleMagnetCancel()
{
	ReleaseMagnet();
}

void AMyPlayerCharacter::HandleMagnetDistance(float AxisValue)
{
	if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent() || FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	MagnetDistance = FMath::Clamp(MagnetDistance + AxisValue * 100.0f, MagnetMinDistance, MagnetMaxDistance);
}

void AMyPlayerCharacter::ReleaseMagnet()
{
	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		PhysicsHandle->ReleaseComponent();
	}

	ClearTargetedMagnetComponent();

	MagnetDistance = FMath::Clamp(MagnetDistance, MagnetMinDistance, MagnetMaxDistance);
	CurrentState = EPlayerState::Normal;
	
	if (AbilityEffect)
	{
		AbilityEffect->ClearVisionEffects();
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Yellow, TEXT("MAGNET MODE: OFF"));
	}
}

void AMyPlayerCharacter::SetPlayerState(const EPlayerState NewState)
{
	if (NewState != EPlayerState::MagnetTargeting && NewState != EPlayerState::MagnetControl)
	{
		ReleaseMagnet();

		return;
	}

	CurrentState = NewState;
}
