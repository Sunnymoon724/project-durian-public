// ReSharper disable All
#include "Abilities/MagnetAbility.h"

#include "Abilities/AbilityEffectComponent.h"
#include "Abilities/AbilityModeSubsystem.h"
#include "Abilities/AbilityReactionComponent.h"
#include "CollisionShape.h"
#include "Constants/GameConstants.h"
#include "Framework/MyPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "Engine/GameInstance.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

FMagnetAbility::FMagnetAbility(AMyPlayerCharacter* InCharacter) : FAbility(InCharacter)
{
}

void FMagnetAbility::Tick(float DeltaTime)
{
	if (!Character)
	{
		return;
	}
	
	EPlayerState state = Character->GetCurrentState();

	switch (state)
	{
	case EPlayerState::MagnetTargeting:
		UpdateTargeting();
		break;
	case EPlayerState::MagnetHolding:
		UpdateControl(DeltaTime);
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(state));
		break;
	}
}

void FMagnetAbility::HandleInteract()
{
	if (Character && Character->GetCurrentState() == EPlayerState::MagnetTargeting)
	{
		SelectTarget();
	}
}

void FMagnetAbility::HandleCancel()
{
	Release();
}

void FMagnetAbility::HandleAbilityUse()
{
	if (!Character)
	{
		return;
	}

	if (Character->GetCurrentState() == EPlayerState::Normal)
	{
		Character->SetPlayerState(EPlayerState::MagnetTargeting);

		if (UGameInstance* GameInstance = Character->GetGameInstance())
		{
			GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityMode::Magnet, true);
		}

		if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
		{
			AbilityEffect->SetVisionEnabled(EAbilityType::Magnet, true);
		}
	}
	else
	{
		Release();
	}
}

void FMagnetAbility::HandleDistance(const float AxisValue)
{
	if (Character && Character->GetCurrentState() == EPlayerState::MagnetHolding && !FMath::IsNearlyZero(AxisValue))
	{
		MagnetDistance = FMath::Clamp(MagnetDistance + AxisValue * 100.0f, MagnetMinDistance, MagnetMaxDistance);
	}
}

void FMagnetAbility::UpdateTargeting()
{
	UPrimitiveComponent* HitComponent = nullptr;
	FVector HitLocation = FVector::ZeroVector;

	if (TraceTarget(HitComponent, HitLocation))
	{
		SetTargetedComponent(HitComponent, HitLocation);

		return;
	}

	ClearTargetedComponent();
}

void FMagnetAbility::UpdateControl(const float DeltaTime)
{
	UPhysicsHandleComponent* PhysicsHandle = Character ? Character->GetPhysicsHandle() : nullptr;

	if (!Character || !PhysicsHandle || !Character->GetController())
	{
		Release();
		return;
	}

	if (!PhysicsHandle->GetGrabbedComponent())
	{
		Character->SetPlayerState(EPlayerState::Normal);
		if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
		{
			AbilityEffect->ClearMagnetHoldLink();
		}
		return;
	}

	const FVector HoldLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	// Use the controller view (mouse) direction, not the pawn body's forward
	// direction, so a held object follows camera yaw and pitch.
	const FVector TargetLocation = HoldLocation + Character->GetController()->GetControlRotation().Vector() * MagnetDistance;
	FVector SafeTargetLocation = TargetLocation;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagnetTargetSweep), false, Character);
	QueryParams.AddIgnoredActor(PhysicsHandle->GetGrabbedComponent()->GetOwner());

	FHitResult SweepHit;
	if (Character->GetWorld()->SweepSingleByChannel(SweepHit, HoldLocation, TargetLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(40.0f), QueryParams))
	{
		SafeTargetLocation = SweepHit.Location;
	}

	// Move the physics-handle target gradually so the object follows with a soft lag.
	CurrentHoldLocation = FMath::VInterpTo(CurrentHoldLocation, SafeTargetLocation, DeltaTime, MagnetFollowSpeed);
	PhysicsHandle->SetTargetLocation(CurrentHoldLocation);

	if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
	{
		AbilityEffect->UpdateMagnetHoldLink(PhysicsHandle->GetGrabbedComponent(), HoldLocation, CurrentHoldLocation);
	}
}

void FMagnetAbility::SelectTarget()
{
	UPhysicsHandleComponent* PhysicsHandle = Character ? Character->GetPhysicsHandle() : nullptr;
	UPrimitiveComponent* HitComponent = nullptr;
	FVector HitLocation = FVector::ZeroVector;

	if (!Character || !PhysicsHandle || !TraceTarget(HitComponent, HitLocation))
	{
		ExitTargetingMode();
		if (Character)
		{
			Character->SetPlayerState(EPlayerState::Normal);
		}
		return;
	}

	PhysicsHandle->GrabComponentAtLocation(HitComponent, NAME_None, HitLocation);
	ExitTargetingMode();

	if (PhysicsHandle->GetGrabbedComponent())
	{
		CurrentHoldLocation = HitLocation;
		const FVector HoldLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
		MagnetDistance = FMath::Clamp(FVector::DotProduct(HitLocation - HoldLocation, Character->GetController()->GetControlRotation().Vector()), MagnetMinDistance, MagnetMaxDistance);
		Character->SetPlayerState(EPlayerState::MagnetHolding);
	}
	else
	{
		Character->SetPlayerState(EPlayerState::Normal);
	}
}

void FMagnetAbility::ExitTargetingMode()
{
	if (!Character)
	{
		return;
	}

	ClearTargetedComponent();
	if (UGameInstance* GameInstance = Character->GetGameInstance())
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityMode::Magnet, false);
	}
	if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
	{
		AbilityEffect->ClearVisionEffects();
	}
}

bool FMagnetAbility::TraceTarget(UPrimitiveComponent*& OutComponent, FVector& OutLocation) const
{
	OutComponent = nullptr;
	OutLocation = FVector::ZeroVector;

	if (!Character || !Character->GetController() || !Character->GetWorld())
	{
		return false;
	}

	// Start at the pawn's eye/view location so the trace does not originate from
	// the third-person camera behind the character mesh.
	const FVector TraceStart = Character->GetPawnViewLocation();
	const FRotator ViewRotation = Character->GetController()->GetControlRotation();
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * Constants::MagnetTargetRange;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagnetTargetTrace), false, Character);
	QueryParams.AddIgnoredActor(Character);

	if (!Character->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return false;
	}

	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	const UAbilityReactionComponent* Reaction = HitResult.GetActor() ? HitResult.GetActor()->FindComponentByClass<UAbilityReactionComponent>() : nullptr;

	if (!Reaction || Reaction->GetReactionType() != EAbilityReactionType::MagnetTarget || !HitComponent || !HitComponent->IsSimulatingPhysics())
	{
		return false;
	}

	OutComponent = HitComponent;
	OutLocation = HitResult.ImpactPoint;
	return true;
}

void FMagnetAbility::Release()
{
	if (!Character)
	{
		return;
	}

	if (UPhysicsHandleComponent* PhysicsHandle = Character->GetPhysicsHandle())
	{
		if (PhysicsHandle->GetGrabbedComponent())
		{
			PhysicsHandle->ReleaseComponent();
		}
	}

	ClearTargetedComponent();

	CurrentHoldLocation = FVector::ZeroVector;
	Character->SetPlayerState(EPlayerState::Normal);

	if (UGameInstance* GameInstance = Character->GetGameInstance())
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityMode::Magnet, false);
	}

	if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
	{
		AbilityEffect->ClearMagnetHoldLink();
		AbilityEffect->ClearVisionEffects();
	}
}

void FMagnetAbility::SetTargetedComponent(UPrimitiveComponent* NewTarget, const FVector& NewTargetLocation)
{
	if (TargetedComponent.IsValid() && TargetedComponent.Get() != NewTarget)
	{
		TargetedComponent->SetCustomDepthStencilValue(1);
	}

	TargetedComponent = NewTarget;
	TargetedLocation = NewTargetLocation;

	if (NewTarget)
	{
		NewTarget->SetRenderCustomDepth(true);
		NewTarget->SetCustomDepthStencilValue(2);
	}
}

void FMagnetAbility::ClearTargetedComponent()
{
	if (TargetedComponent.IsValid())
	{
		TargetedComponent->SetCustomDepthStencilValue(1);
	}

	TargetedComponent.Reset();
	TargetedLocation = FVector::ZeroVector;
}
