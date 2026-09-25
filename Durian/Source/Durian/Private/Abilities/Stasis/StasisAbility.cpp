#include "Abilities/Stasis/StasisAbility.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "Abilities/Core/AbilityModeSubsystem.h"
#include "Abilities/Stasis/StasisTargetComponent.h"
#include "Constants/GameConstantsDataAsset.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"

void FStasisAbility::Tick(float DeltaTime)
{
	if (!Character)
	{
		return;
	}
	CooldownRemaining = FMath::Max(0.0f, CooldownRemaining - DeltaTime);
	if (IsStasisActive())
	{
		RemainingTime -= DeltaTime;
		if (RemainingTime <= 0.0f)
		{
			EndStasis(true, true);
		}
		else if (UStasisTargetComponent* Target = ActiveTarget.Get())
		{
			Target->UpdateFeedback(RemainingTime, UGameConstantsDataAsset::Get()->StasisDuration, Character->GetPawnViewLocation(), UGameConstantsDataAsset::Get()->StasisMaxImpulse);
		}
	}
	else if (ActiveTarget.IsValid())
	{
		EndStasis(false, false);
	}
	if (Character->GetCurrentAbilityType() == EAbilityType::Stasis && Character->GetCurrentState() == EPlayerState::StasisTargeting)
	{
		if (Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling())
		{
			HandleCancel();
			return;
		}
		UpdateTargeting();
	}
}

void FStasisAbility::HandleInteract()
{
	if (Character && Character->GetCurrentState() == EPlayerState::StasisTargeting)
	{
		StartStasis();
	}
}

void FStasisAbility::HandleCancel()
{
	if (!Character || Character->GetCurrentState() != EPlayerState::StasisTargeting)
	{
		return;
	}
	ClearTarget();
	Character->SetPlayerState(EPlayerState::Normal);
	if (UGameInstance* GameInstance = Character->GetGameInstance())
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityVisualMode::Stasis, false);
	}
	if (UAbilityEffectComponent* Effect = Character->GetAbilityEffect())
	{
		Effect->SetVisionEnabled(EAbilityType::Stasis, false);
	}
}

void FStasisAbility::HandleAbilityUse()
{
	if (!Character || IsStasisActive() || CooldownRemaining > 0.0f)
	{
		return;
	}
	if (Character->GetCurrentState() == EPlayerState::StasisTargeting)
	{
		HandleCancel();
		return;
	}
	if (Character->GetCurrentState() != EPlayerState::Normal)
	{
		return;
	}
	Character->SetPlayerState(EPlayerState::StasisTargeting);
	if (UGameInstance* GameInstance = Character->GetGameInstance())
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityVisualMode::Stasis, true);
	}
	if (UAbilityEffectComponent* Effect = Character->GetAbilityEffect())
	{
		Effect->SetVisionEnabled(EAbilityType::Stasis, true);
	}
}

void FStasisAbility::HandleAttackHit(const FHitResult& Hit, const FVector& AttackDirection)
{
	UStasisTargetComponent* Target = ActiveTarget.Get();
	if (!Target || !Target->IsStasisActive() || Hit.GetComponent() != Target->GetFrozenPrimitive())
	{
		return;
	}
	Target->AccumulateImpulse(AttackDirection.GetSafeNormal() * UGameConstantsDataAsset::Get()->StasisImpulsePerHit, UGameConstantsDataAsset::Get()->StasisMaxImpulse);
}

void FStasisAbility::UpdateTargeting()
{
	UPrimitiveComponent* NewTarget = nullptr;
	if (!TraceTarget(NewTarget))
	{
		ClearTarget();
		return;
	}
	if (TargetedComponent.Get() != NewTarget)
	{
		ClearTarget();
		if (UClass* MarkerClass = LoadClass<AActor>(nullptr, TEXT("/Game/Resources/VFX/Stasis/Blueprints/BP_StasisTargetVFX.BP_StasisTargetVFX_C")))
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = NewTarget->GetOwner();
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AimingMarker = Character->GetWorld()->SpawnActor<AActor>(MarkerClass, NewTarget->Bounds.Origin + FVector(0.0f, 0.0f, NewTarget->Bounds.BoxExtent.Z + 20.0f), FRotator::ZeroRotator, SpawnParameters);
			if (AActor* Marker = AimingMarker.Get())
			{
				Marker->SetActorEnableCollision(false);
				Marker->AttachToComponent(NewTarget, FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}
	TargetedComponent = NewTarget;
	if (UAbilityReactionComponent* Reaction = NewTarget->GetOwner()->FindComponentByClass<UAbilityReactionComponent>())
	{
		Reaction->SetAimedTarget(true);
	}
}

void FStasisAbility::StartStasis()
{
	if (IsStasisActive())
	{
		return;
	}
	UPrimitiveComponent* Primitive = nullptr;
	if (!TraceTarget(Primitive))
	{
		HandleCancel();
		return;
	}
	AActor* TargetActor = Primitive->GetOwner();
	UStasisTargetComponent* Target = TargetActor->FindComponentByClass<UStasisTargetComponent>();
	if (!Target)
	{
		Target = NewObject<UStasisTargetComponent>(TargetActor);
		TargetActor->AddInstanceComponent(Target);
		Target->RegisterComponent();
	}
	if (!Target->BeginStasis(Primitive))
	{
		HandleCancel();
		return;
	}
	ActiveTarget = Target;
	RemainingTime = UGameConstantsDataAsset::Get()->StasisDuration;
	ClearTarget();
	Character->SetPlayerState(EPlayerState::StasisActive);
	if (UGameInstance* GameInstance = Character->GetGameInstance())
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->SetAbilityModeActive(EAbilityVisualMode::Stasis, false);
	}
	if (UAbilityEffectComponent* Effect = Character->GetAbilityEffect())
	{
		Effect->SetVisionEnabled(EAbilityType::Stasis, false);
	}
	UE_LOG(LogTemp, Log, TEXT("Stasis started on %s"), *GetNameSafe(TargetActor));
}

void FStasisAbility::EndStasis(const bool bApplyImpulse, const bool bStartCooldown)
{
	if (UStasisTargetComponent* Target = ActiveTarget.Get())
	{
		Target->EndStasis(bApplyImpulse);
	}
	ActiveTarget.Reset();
	RemainingTime = 0.0f;
	if (bStartCooldown)
	{
		CooldownRemaining = UGameConstantsDataAsset::Get()->StasisCooldown;
	}
	if (Character && Character->GetCurrentState() == EPlayerState::StasisActive)
	{
		Character->SetPlayerState(EPlayerState::Normal);
	}
}

void FStasisAbility::ClearTarget()
{
	if (AActor* Marker = AimingMarker.Get())
	{
		Marker->Destroy();
	}
	AimingMarker.Reset();
	if (UPrimitiveComponent* Primitive = TargetedComponent.Get())
	{
		if (UAbilityReactionComponent* Reaction = Primitive->GetOwner()->FindComponentByClass<UAbilityReactionComponent>())
		{
			Reaction->SetAimedTarget(false);
		}
	}
	TargetedComponent.Reset();
}

void FStasisAbility::AbortForEndPlay()
{
	if (!Character)
	{
		return;
	}
	if (Character->GetCurrentState() == EPlayerState::StasisTargeting)
	{
		HandleCancel();
	}
	if (ActiveTarget.IsValid())
	{
		EndStasis(false, false);
	}
}

bool FStasisAbility::IsStasisActive() const
{
	const UStasisTargetComponent* Target = ActiveTarget.Get();
	return Target && Target->IsStasisActive();
}

FVector FStasisAbility::GetAccumulatedImpulse() const
{
	const UStasisTargetComponent* Target = ActiveTarget.Get();
	return Target ? Target->GetAccumulatedImpulse() : FVector::ZeroVector;
}

bool FStasisAbility::TraceTarget(UPrimitiveComponent*& OutComponent) const
{
	FHitResult Hit;
	if (!TraceAbilityTarget(EAbilityReactionType::StasisTarget, UGameConstantsDataAsset::Get()->StasisTargetRange, Hit, false, AimingMarker.Get(), true))
	{
		OutComponent = nullptr;
		return false;
	}

	OutComponent = Hit.GetComponent();
	return true;
}
