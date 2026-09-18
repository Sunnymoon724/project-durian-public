// ReSharper disable All
#include "Abilities/Magnesis/MagnesisAbility.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Core/AbilityModeSubsystem.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "CollisionShape.h"
#include "Constants/GameConstants.h"
#include "DrawDebugHelpers.h"
#include "Framework/MyPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "Engine/GameInstance.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

FMagnesisAbility::FMagnesisAbility(AMyPlayerCharacter* InCharacter) : FAbility(InCharacter)
{
}

void FMagnesisAbility::Tick(float DeltaTime)
{
	if (!Character)
	{
		return;
	}
	
	EPlayerState state = Character->GetCurrentState();

	switch (state)
	{
	case EPlayerState::MagnesisTargeting:
		UpdateTargeting();
		break;
	case EPlayerState::MagnesisHolding:
		UpdateControl(DeltaTime);
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(state));
		break;
	}
}

void FMagnesisAbility::HandleInteract()
{
	if (Character && Character->GetCurrentState() == EPlayerState::MagnesisTargeting)
	{
		SelectTarget();
	}
}

void FMagnesisAbility::HandleCancel()
{
	Release();
}

void FMagnesisAbility::HandleAbilityUse()
{
	if (!Character)
	{
		return;
	}

	if (Character->GetCurrentState() == EPlayerState::Normal)
	{
		Character->SetPlayerState(EPlayerState::MagnesisTargeting);

		if (UGameInstance* GameInstance = Character->GetGameInstance())
		{
			if (UAbilityModeSubsystem* AbilityModeSubsystem = GameInstance->GetSubsystem<UAbilityModeSubsystem>())
			{
				const FVector ViewDirection = Character->GetController()
					? Character->GetController()->GetControlRotation().Vector()
					: Character->GetActorForwardVector();
				AbilityModeSubsystem->SetModeScanDirection(FVector2D(ViewDirection.X, ViewDirection.Y));
				AbilityModeSubsystem->SetAbilityModeActive(EAbilityMode::Magnesis, true);
			}
		}

		if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
		{
			AbilityEffect->SetVisionEnabled(EAbilityType::Magnesis, true);
		}
	}
	else
	{
		Release();
	}
}

void FMagnesisAbility::HandleDistance(const float AxisValue)
{
	if (Character && Character->GetCurrentState() == EPlayerState::MagnesisHolding && !FMath::IsNearlyZero(AxisValue))
	{
		MagnesisDistance = FMath::Clamp(MagnesisDistance + AxisValue * 100.0f, MagnesisMinDistance, MagnesisMaxDistance);
	}
}

void FMagnesisAbility::UpdateTargeting()
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

void FMagnesisAbility::UpdateControl(const float DeltaTime)
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
			AbilityEffect->ClearMagnesisHoldLink();
		}
		return;
	}

	const FVector HoldLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	// Use the controller view (mouse) direction, not the pawn body's forward
	// direction, so a held object follows camera yaw and pitch.
	const FVector TargetLocation = HoldLocation + Character->GetController()->GetControlRotation().Vector() * MagnesisDistance;
	FVector SafeTargetLocation = TargetLocation;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagnesisTargetSweep), false, Character);
	QueryParams.AddIgnoredActor(PhysicsHandle->GetGrabbedComponent()->GetOwner());

	FHitResult SweepHit;
	if (Character->GetWorld()->SweepSingleByChannel(SweepHit, HoldLocation, TargetLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(40.0f), QueryParams))
	{
		SafeTargetLocation = SweepHit.Location;
	}

	// Move the physics-handle target gradually so the object follows with a soft lag.
	CurrentHoldLocation = FMath::VInterpTo(CurrentHoldLocation, SafeTargetLocation, DeltaTime, MagnesisFollowSpeed);
	PhysicsHandle->SetTargetLocation(CurrentHoldLocation);

	if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
	{
		AbilityEffect->UpdateMagnesisHoldLink(PhysicsHandle->GetGrabbedComponent(), HoldLocation, CurrentHoldLocation);
	}
}

void FMagnesisAbility::SelectTarget()
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
		MagnesisDistance = FMath::Clamp(FVector::DotProduct(HitLocation - HoldLocation, Character->GetController()->GetControlRotation().Vector()), MagnesisMinDistance, MagnesisMaxDistance);
		Character->SetPlayerState(EPlayerState::MagnesisHolding);
	}
	else
	{
		Character->SetPlayerState(EPlayerState::Normal);
	}
}

void FMagnesisAbility::ExitTargetingMode()
{
	if (!Character)
	{
		return;
	}

	ClearTargetedComponent();
	if (UGameInstance* GameInstance = Character->GetGameInstance())
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityMode::Magnesis, false);
	}
	if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
	{
		AbilityEffect->ClearVisionEffects();
	}
}

bool FMagnesisAbility::TraceTarget(UPrimitiveComponent*& OutComponent, FVector& OutLocation) const
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
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * Constants::MagnesisTargetRange;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagnesisTargetTrace), false, Character);
	QueryParams.AddIgnoredActor(Character);

	const bool bTraceHit = Character->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	const UAbilityReactionComponent* Reaction = bTraceHit && HitResult.GetActor()
		? HitResult.GetActor()->FindComponentByClass<UAbilityReactionComponent>()
		: nullptr;
	const bool bValidTarget = Reaction && Reaction->GetReactionType() == EAbilityReactionType::MagnesisTarget;

	DrawDebugLine(
		Character->GetWorld(),
		TraceStart,
		bTraceHit ? HitResult.ImpactPoint : TraceEnd,
		bValidTarget ? FColor::Green : bTraceHit ? FColor::Yellow : FColor::Red,
		false,
		0.1f,
		1,
		2.0f);
	if (bTraceHit)
	{
		DrawDebugPoint(
			Character->GetWorld(),
			HitResult.ImpactPoint,
			12.0f,
			bValidTarget ? FColor::Green : FColor::Yellow,
			false,
			0.1f,
			1);
	}

	if (!bTraceHit || !bValidTarget)
	{
		return false;
	}

	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	OutComponent = HitComponent;
	OutLocation = HitResult.ImpactPoint;

	return true;
}

void FMagnesisAbility::Release()
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
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityMode::Magnesis, false);
	}

	if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
	{
		AbilityEffect->ClearMagnesisHoldLink();
		AbilityEffect->ClearVisionEffects();
	}
}

void FMagnesisAbility::SetTargetedComponent(UPrimitiveComponent* NewTarget, const FVector& NewTargetLocation)
{
	if (TargetedComponent.IsValid() && TargetedComponent.Get() != NewTarget)
	{
		TargetedComponent->SetCustomDepthStencilValue(1);
		if (UAbilityReactionComponent* PreviousReaction = TargetedComponent->GetOwner()
			? TargetedComponent->GetOwner()->FindComponentByClass<UAbilityReactionComponent>()
			: nullptr)
		{
			PreviousReaction->SetAimedTarget(false);
		}
	}

	TargetedComponent = NewTarget;
	TargetedLocation = NewTargetLocation;

	if (NewTarget)
	{
		NewTarget->SetRenderCustomDepth(true);
		NewTarget->SetCustomDepthStencilValue(2);
		if (UAbilityReactionComponent* Reaction = NewTarget->GetOwner()
			? NewTarget->GetOwner()->FindComponentByClass<UAbilityReactionComponent>()
			: nullptr)
		{
			Reaction->SetAimedTarget(true);
		}
	}
}

void FMagnesisAbility::ClearTargetedComponent()
{
	if (TargetedComponent.IsValid())
	{
		TargetedComponent->SetCustomDepthStencilValue(1);
		if (UAbilityReactionComponent* Reaction = TargetedComponent->GetOwner()
			? TargetedComponent->GetOwner()->FindComponentByClass<UAbilityReactionComponent>()
			: nullptr)
		{
			Reaction->SetAimedTarget(false);
		}
	}

	TargetedComponent.Reset();
	TargetedLocation = FVector::ZeroVector;
}
