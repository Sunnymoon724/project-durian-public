// ReSharper disable All
#include "Abilities/Magnesis/MagnesisAbility.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Core/AbilityModeSubsystem.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "CollisionShape.h"
#include "Constants/GameConstantsDataAsset.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/GameInstance.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

FMagnesisAbility::FMagnesisAbility(AKzPlayerCharacter* InCharacter) : FAbility(InCharacter)
{
	MagnesisDistance = UGameConstantsDataAsset::Get()->MagnesisDefaultDistance;
}

void FMagnesisAbility::Tick(float DeltaTime)
{
	if (!Character)
	{
		return;
	}
	
	EPlayerState state = Character->GetCurrentState();
	if ((state == EPlayerState::MagnesisTargeting || state == EPlayerState::MagnesisHolding) && Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling())
	{
		Release();
		return;
	}

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
				const FVector ViewDirection = Character->GetController() ? Character->GetController()->GetControlRotation().Vector() : Character->GetActorForwardVector();
				AbilityModeSubsystem->SetModeScanDirection(FVector2D(ViewDirection.X, ViewDirection.Y));
				AbilityModeSubsystem->SetAbilityModeActive(EAbilityVisualMode::Magnesis, true);
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
		MagnesisDistance = FMath::Clamp(MagnesisDistance + AxisValue * 100.0f, UGameConstantsDataAsset::Get()->MagnesisMinDistance, UGameConstantsDataAsset::Get()->MagnesisMaxDistance);
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
	CurrentHoldLocation = FMath::VInterpTo(CurrentHoldLocation, SafeTargetLocation, DeltaTime, UGameConstantsDataAsset::Get()->MagnesisFollowSpeed);
	PhysicsHandle->SetTargetLocation(CurrentHoldLocation);

}

void FMagnesisAbility::SelectTarget()
{
	UPhysicsHandleComponent* PhysicsHandle = Character ? Character->GetPhysicsHandle() : nullptr;
	UPrimitiveComponent* HitComponent = nullptr;
	FVector HitLocation = FVector::ZeroVector;

	if (!Character || !PhysicsHandle)
	{
		return;
	}
	if (!TraceTarget(HitComponent, HitLocation))
	{
		ExitTargetingMode();
		Character->SetPlayerState(EPlayerState::Normal);
		return;
	}

	PhysicsHandle->GrabComponentAtLocation(HitComponent, NAME_None, HitLocation);
	ExitTargetingMode();

	if (PhysicsHandle->GetGrabbedComponent())
	{
		if (UNiagaraSystem* GrabPulse = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Resources/VFX/Magnesis/Niagara/NS_MagnesisGrabPulse.NS_MagnesisGrabPulse")))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(Character->GetWorld(), GrabPulse, HitComponent->Bounds.Origin);
		}
		CurrentHoldLocation = HitLocation;

		const FVector HoldLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);

		MagnesisDistance = FMath::Clamp(FVector::DotProduct(HitLocation - HoldLocation, Character->GetController()->GetControlRotation().Vector()), UGameConstantsDataAsset::Get()->MagnesisMinDistance, UGameConstantsDataAsset::Get()->MagnesisMaxDistance);

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
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityVisualMode::Magnesis, false);
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

	FHitResult HitResult;
	const bool bValidTarget = TraceAbilityTarget(EAbilityReactionType::MagnesisTarget, UGameConstantsDataAsset::Get()->MagnesisTargetRange, HitResult, false, nullptr, true);
	const FVector TraceStart = Character ? Character->GetPawnViewLocation() : FVector::ZeroVector;
	const FVector TraceEnd = Character && Character->GetController() ? TraceStart + Character->GetController()->GetControlRotation().Vector() * UGameConstantsDataAsset::Get()->MagnesisTargetRange : TraceStart;

	const bool bTraceHit = HitResult.bBlockingHit;
	if (Character && Character->GetWorld()) DrawDebugLine(Character->GetWorld(),TraceStart,bTraceHit ? HitResult.ImpactPoint : TraceEnd,bValidTarget ? FColor::Green : bTraceHit ? FColor::Yellow : FColor::Red,false,0.1f,1,2.0f);

	if (bTraceHit && Character && Character->GetWorld())
	{
		DrawDebugPoint(Character->GetWorld(),HitResult.ImpactPoint,12.0f,bValidTarget ? FColor::Green : FColor::Yellow,false,0.1f,1);
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
			const FVector ReleaseLocation = PhysicsHandle->GetGrabbedComponent()->Bounds.Origin;
			PhysicsHandle->ReleaseComponent();
			if (UNiagaraSystem* ReleasePulse = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Resources/VFX/Magnesis/Niagara/NS_MagnesisReleasePulse.NS_MagnesisReleasePulse")))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(Character->GetWorld(), ReleasePulse, ReleaseLocation);
			}
		}
	}

	ClearTargetedComponent();

	CurrentHoldLocation = FVector::ZeroVector;
	Character->SetPlayerState(EPlayerState::Normal);

	if (UGameInstance* GameInstance = Character->GetGameInstance())
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityVisualMode::Magnesis, false);
	}

	if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
	{
		AbilityEffect->ClearVisionEffects();
	}
}

void FMagnesisAbility::SetTargetedComponent(UPrimitiveComponent* NewTarget, const FVector& NewTargetLocation)
{
	if (TargetedComponent.IsValid() && TargetedComponent.Get() != NewTarget)
	{
		TargetedComponent->SetCustomDepthStencilValue(1);

		if (UAbilityReactionComponent* PreviousReaction = TargetedComponent->GetOwner() ? TargetedComponent->GetOwner()->FindComponentByClass<UAbilityReactionComponent>() : nullptr)
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

		if (UAbilityReactionComponent* Reaction = NewTarget->GetOwner() ? NewTarget->GetOwner()->FindComponentByClass<UAbilityReactionComponent>() : nullptr)
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

		if (UAbilityReactionComponent* Reaction = TargetedComponent->GetOwner()? TargetedComponent->GetOwner()->FindComponentByClass<UAbilityReactionComponent>() : nullptr)
		{
			Reaction->SetAimedTarget(false);
		}
	}

	TargetedComponent.Reset();
	TargetedLocation = FVector::ZeroVector;
}
