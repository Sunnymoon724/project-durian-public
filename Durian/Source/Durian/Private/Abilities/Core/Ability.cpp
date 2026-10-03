#include "Abilities/Core/Ability.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Core/AbilityModeSubsystem.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Engine/GameInstance.h"

float FAbility::GetRemainingCooldown(const double CooldownEndTime) const
{
	const double CurrentTime = Character && Character->GetWorld() ? Character->GetWorld()->GetTimeSeconds() : 0.0;
	return static_cast<float>(FMath::Max(0.0, CooldownEndTime - CurrentTime));
}

void FAbility::SetAbilityModeActive(const EAbilityVisualMode Mode, const bool bActive, const bool bUpdateScanDirection) const
{
	if (!Character)
	{
		return;
	}

	const UGameInstance* GameInstance = Character->GetGameInstance();
	UAbilityModeSubsystem* AbilityModeSubsystem = GameInstance ? GameInstance->GetSubsystem<UAbilityModeSubsystem>() : nullptr;
	if (!AbilityModeSubsystem)
	{
		return;
	}

	if (bActive && bUpdateScanDirection)
	{
		const FVector ViewDirection = Character->GetController() ? Character->GetController()->GetControlRotation().Vector() : Character->GetActorForwardVector();
		AbilityModeSubsystem->SetModeScanDirection(FVector2D(ViewDirection.X, ViewDirection.Y));
	}

	AbilityModeSubsystem->SetAbilityModeActive(Mode, bActive);
}

void FAbility::SetAbilityVisionEnabled(const EAbilityType Ability, const bool bEnabled) const
{
	if (Character)
	{
		if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
		{
			AbilityEffect->SetVisionEnabled(Ability, bEnabled);
		}
	}
}

bool FAbility::UpdateScanActivation(const EAbilityVisualMode Mode, const EAbilityType Ability, const bool bUpdateScanDirection) const
{
	if (!Character || !Character->IsScanPoseReady()) return false;
	const UGameInstance* GameInstance = Character->GetGameInstance();
	const UAbilityModeSubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<UAbilityModeSubsystem>() : nullptr;
	if (Subsystem && !Subsystem->IsAbilityModeActive(Mode))
	{
		// Called only while targeting. Cancelling/changing abilities during Enter
		// cannot leave a delayed timer that later switches the scan back on.
		SetAbilityModeActive(Mode, true, bUpdateScanDirection);
		SetAbilityVisionEnabled(Ability, true);
	}
	return true;
}

void FAbility::ClearAbilityVisionEffects() const
{
	if (Character)
	{
		if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
		{
			AbilityEffect->ClearVisionEffects();
		}
	}
}

bool FAbility::TraceAbilityTarget(const EAbilityReactionType ReactionType, const float TraceRange, FHitResult& OutHit, const bool bTargetAtFeet, const AActor* IgnoredActor, const bool bRequirePhysics) const
{
	if (!Character || !Character->GetWorld() || !Character->GetController())
	{
		return false;
	}

	FVector TraceStart = Character->GetPawnViewLocation();
	FVector TraceEnd = TraceStart + Character->GetController()->GetControlRotation().Vector() * TraceRange;

	if (bTargetAtFeet)
	{
		TraceStart = Character->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
		TraceEnd = Character->GetActorLocation() - FVector(0.0f, 0.0f, 200.0f);
	}

	// Swimming puts the pawn's eye/feet probe below the water plane. A downward
	// trace would then see the pool floor instead of the Cryonis surface.
	// Keep other abilities and grounded aiming unchanged; still require a real
	// visible CryonicTarget hit rather than accepting the PhysicsVolume itself.
	if (ReactionType == EAbilityReactionType::CryonicTarget
		&& Character->GetCharacterMovement()->IsSwimming())
	{
		const APhysicsVolume* Water = Character->GetPhysicsVolume();
		if (Water && Water->bWaterVolume)
		{
			FVector Origin, Extent;
			Water->GetActorBounds(false, Origin, Extent);
			TraceStart.Z = FMath::Max(TraceStart.Z, Origin.Z + Extent.Z + 50.0f);
			if (!bTargetAtFeet)
			{
				TraceEnd = TraceStart + Character->GetController()->GetControlRotation().Vector() * TraceRange;
			}
		}
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AbilityTargetTrace), false, Character);

	QueryParams.AddIgnoredActor(Character);

	if (IgnoredActor)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}

	const bool bTraceHit = Character->GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	const AActor* HitActor = bTraceHit ? OutHit.GetActor() : nullptr;
	const UAbilityReactionComponent* Reaction = HitActor ? HitActor->FindComponentByClass<UAbilityReactionComponent>() : nullptr;
	const UPrimitiveComponent* HitComponent = bTraceHit ? OutHit.GetComponent() : nullptr;
	const bool bValidTarget = bTraceHit && Reaction && Reaction->GetReactionType() == ReactionType && HitComponent && (!bRequirePhysics || HitComponent->IsSimulatingPhysics());
	const FVector DebugEnd = bTraceHit ? OutHit.ImpactPoint : TraceEnd;
	const FColor DebugColor = bValidTarget ? FColor::Green : bTraceHit ? FColor::Yellow : FColor::Red;

	DrawDebugLine(Character->GetWorld(), TraceStart, DebugEnd, DebugColor, false, 0.1f, 1, 2.0f);

	if (bTraceHit)
	{
		DrawDebugPoint(Character->GetWorld(), OutHit.ImpactPoint, 12.0f, DebugColor, false, 0.1f, 1);
	}

	return bValidTarget;
}
