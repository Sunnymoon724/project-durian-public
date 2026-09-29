#include "Abilities/Stasis/StasisAbility.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "Abilities/Stasis/StasisTargetComponent.h"
#include "Constants/GameConstantsDataAsset.h"
#include "Enums/PlayerEnums.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"

void FStasisAbility::Tick(float)
{
	if (!Character)
	{
		return;
	}

	if (Character->GetCurrentAbilityType() == EAbilityType::Stasis && Character->GetCurrentState() == EPlayerState::StasisTargeting)
	{
		if (Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling())
		{
			HandleInput(EAbilityInput::Cancel);

			return;
		}

		UpdateTargeting();
	}
}

void FStasisAbility::TickPersistent(float DeltaTime)
{
	if (!Character)
	{
		return;
	}

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
}

float FStasisAbility::GetCooldownRemaining() const
{
	return GetRemainingCooldown(CooldownEndTime);
}

void FStasisAbility::HandleInput(const EAbilityInput Input, float)
{
	if (!Character)
	{
		return;
	}

	switch (Input)
	{
	case EAbilityInput::Interact:
		{
			if (Character->GetCurrentState() == EPlayerState::StasisTargeting)
			{
				StartStasis();
			}
			break;
		}
	case EAbilityInput::Cancel:
	case EAbilityInput::Interrupt:
		{
			if (Character->GetCurrentState() == EPlayerState::StasisTargeting)
			{
				ClearTarget();
				Character->SetPlayerState(EPlayerState::Normal);
				SetAbilityModeActive(EAbilityVisualMode::Stasis, false);
				SetAbilityVisionEnabled(EAbilityType::Stasis, false);
			}

			break;
		}
	case EAbilityInput::Use:
		{
			if (IsStasisActive() || GetCooldownRemaining() > 0.0f)
			{
				return;
			}

			if (Character->GetCurrentState() == EPlayerState::StasisTargeting)
			{
				HandleInput(EAbilityInput::Cancel);
				return;
			}

			if (Character->GetCurrentState() != EPlayerState::Normal)
			{
				return;
			}

			Character->SetPlayerState(EPlayerState::StasisTargeting);
			SetAbilityModeActive(EAbilityVisualMode::Stasis, true);
			SetAbilityVisionEnabled(EAbilityType::Stasis, true);

			break;
		}
	default:
		break;
	}
}

void FStasisAbility::HandleAbilityDeselected()
{
	HandleInput(EAbilityInput::Cancel);

	if (Character && IsStasisActive() && Character->GetCurrentState() == EPlayerState::StasisActive)
	{
		Character->SetPlayerState(EPlayerState::Normal);
	}
}

void FStasisAbility::HandleAttackHit(const FHitResult& Hit, const FVector& AttackDirection) const
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
		HandleInput(EAbilityInput::Cancel);

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
		HandleInput(EAbilityInput::Cancel);

		return;
	}

	ActiveTarget = Target;
	RemainingTime = UGameConstantsDataAsset::Get()->StasisDuration;
	ClearTarget();
	Character->SetPlayerState(EPlayerState::StasisActive);
	SetAbilityModeActive(EAbilityVisualMode::Stasis, false);
	SetAbilityVisionEnabled(EAbilityType::Stasis, false);

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
		CooldownEndTime = Character && Character->GetWorld() ? Character->GetWorld()->GetTimeSeconds() + UGameConstantsDataAsset::Get()->StasisCooldown : 0.0;
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
		HandleInput(EAbilityInput::Cancel);
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
