#include "Abilities/Cryonis/CryonisAbility.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Core/AbilityModeSubsystem.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "Abilities/Cryonis/IcePillar.h"
#include "Abilities/Cryonis/IcePlacementPreview.h"
#include "Engine/GameInstance.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/PrimitiveComponent.h"

void FCryonisAbility::Tick(const float DeltaTime)
{
	if (!Character || Character->GetCurrentState() != EPlayerState::IceTargeting)
	{
		return;
	}

	SpawnCooldownRemaining = FMath::Max(0.0f, SpawnCooldownRemaining - DeltaTime);

	UpdateTargeting();
}

void FCryonisAbility::HandleInteract()
{
	if (!Character || Character->GetCurrentState() != EPlayerState::IceTargeting)
	{
		return;
	}

	if (TargetPillar.IsValid())
	{
		RemoveTargetedPillar();
	}
	else if (bTargetValid && SpawnCooldownRemaining <= 0.0f)
	{
		SpawnIcePillar();
	}
}

void FCryonisAbility::HandleCancel()
{
	if (Character && Character->GetCurrentState() == EPlayerState::IceTargeting)
	{
		ExitTargetingMode();
	}
}

void FCryonisAbility::HandleAbilityUse()
{
	if (!Character)
	{
		return;
	}

	if (Character->GetCurrentState() == EPlayerState::Normal)
	{
		Character->SetPlayerState(EPlayerState::IceTargeting);
		bTargetAtFeet = false;
		EnsurePreview();
		
		const UGameInstance* GameInstance = Character->GetGameInstance();

		if (GameInstance)
		{
			if (UAbilityModeSubsystem* AbilityModeSubsystem = GameInstance->GetSubsystem<UAbilityModeSubsystem>())
			{
				const FVector ViewDirection = Character->GetController()
					? Character->GetController()->GetControlRotation().Vector()
					: Character->GetActorForwardVector();
				AbilityModeSubsystem->SetModeScanDirection(FVector2D(ViewDirection.X, ViewDirection.Y));
				AbilityModeSubsystem->SetAbilityModeActive(EAbilityVisualMode::Cryonis, true);
			}
		}

		// The Cryonis world-scan post process currently produces unstable
		// temporal/dithered edges on large water surfaces. Keep the ability-mode
		// target feedback, but do not enable that full-screen effect until its
		// material is corrected.
		if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
		{
			AbilityEffect->SetVisionEnabled(EAbilityType::Cryonis, false);
		}
	}
	else if (Character->GetCurrentState() == EPlayerState::IceTargeting)
	{
		ExitTargetingMode();
	}
}

void FCryonisAbility::HandleTargetAtFeet()
{
	if (Character && Character->GetCurrentState() == EPlayerState::IceTargeting)
	{
		bTargetAtFeet = !bTargetAtFeet;
	}
}

bool FCryonisAbility::IsIcePillarTarget(const AActor* Actor)
{
	return Actor && (Actor->ActorHasTag(TEXT("IcePillar")) || Actor->IsA<AIcePillar>());
}

void FCryonisAbility::UpdateTargeting()
{
	// Clear the current hit result without clearing the aimed actor first.
	// The aimed state is changed only when the traced actor changes.
	TargetSurface.Reset();
	TargetPillar.Reset();
	TargetLocation = FVector::ZeroVector;
	bTargetValid = false;
	EnsurePreview();

	if (!PlacementPreview.IsValid())
	{
		return;
	}

	FHitResult Hit;

	if (!TraceTarget(Hit))
	{
		UpdateAimedTarget(nullptr);
		PlacementPreview->SetPreviewState(FVector::ZeroVector, false, false);

		return;
	}

	AActor* HitActor = Hit.GetActor();

	if (IsIcePillarTarget(HitActor))
	{
		UpdateAimedTarget(HitActor);
		TargetPillar = Cast<AIcePillar>(HitActor);
		PlacementPreview->SetPreviewState(FVector::ZeroVector, false, false);

		return;
	}

	const UAbilityReactionComponent* Reaction = HitActor
		? HitActor->FindComponentByClass<UAbilityReactionComponent>()
		: nullptr;

	if (!HitActor || !Reaction || Reaction->GetReactionType() != EAbilityReactionType::CryonicTarget)
	{
		UpdateAimedTarget(nullptr);
		PlacementPreview->SetPreviewState(FVector::ZeroVector, false, false);

		return;
	}

	TargetSurface = HitActor;
	UpdateAimedTarget(HitActor);
	TargetLocation = GetSpawnLocation(Hit);
	bTargetValid = SpawnCooldownRemaining <= 0.0f && CanSpawnAt(TargetLocation, HitActor);
	PlacementPreview->SetPreviewState(TargetLocation, true, bTargetValid);
}

void FCryonisAbility::SpawnIcePillar()
{
	if (!Character || !Character->GetWorld() || !TargetSurface.IsValid() || !bTargetValid)
	{
		return;
	}

	const FVector SpawnLocation = PlacementPreview.IsValid()
		? PlacementPreview->GetActorLocation()
		: TargetLocation;

	if (!CanSpawnAt(SpawnLocation, TargetSurface.Get()))
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Character;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIcePillar* NewPillar = Character->GetWorld()->SpawnActor<AIcePillar>(SpawnLocation, FRotator::ZeroRotator, SpawnParameters);

	if (!NewPillar)
	{
		return;
	}

	SpawnedPillars.Add(NewPillar);
	SpawnCooldownRemaining = SpawnCooldown;

	if (PlacementPreview.IsValid())
	{
		PlacementPreview->PlaySpawnEffect();
	}

	while (SpawnedPillars.Num() > MaxPillarCount)
	{
		if (AIcePillar* OldestPillar = SpawnedPillars[0].Get())
		{
			DestroyPillar(OldestPillar);
		}
		SpawnedPillars.RemoveAt(0);
	}
}

void FCryonisAbility::RemoveTargetedPillar()
{
	AIcePillar* Pillar = TargetPillar.Get();

	if (!Pillar)
	{
		return;
	}

	DestroyPillar(Pillar);
	SpawnedPillars.Remove(Pillar);
	ClearTarget();
}

void FCryonisAbility::ExitTargetingMode()
{
	if (!Character)
	{
		return;
	}

	ClearTarget();

	if (PlacementPreview.IsValid())
	{
		PlacementPreview->ClearPreview();
	}

	Character->SetPlayerState(EPlayerState::Normal);

	if (UGameInstance* GameInstance = Character->GetGameInstance())
	{
		if (UAbilityModeSubsystem* AbilityModeSubsystem = GameInstance->GetSubsystem<UAbilityModeSubsystem>())
		{
			AbilityModeSubsystem->SetAbilityModeActive(EAbilityVisualMode::Cryonis, false);
		}
	}

	if (UAbilityEffectComponent* AbilityEffect = Character->GetAbilityEffect())
	{
		AbilityEffect->ClearVisionEffects();
	}
}

void FCryonisAbility::ClearTarget()
{
	UpdateAimedTarget(nullptr);
	TargetSurface.Reset();
	TargetPillar.Reset();
	TargetLocation = FVector::ZeroVector;
	bTargetValid = false;
}

void FCryonisAbility::UpdateAimedTarget(AActor* NewTarget)
{
	if (AimedTargetActor.Get() == NewTarget)
	{
		return;
	}

	if (UAbilityReactionComponent* PreviousReaction = AimedTargetActor.IsValid()
		? AimedTargetActor->FindComponentByClass<UAbilityReactionComponent>()
		: nullptr)
	{
		PreviousReaction->SetAimedTarget(false);
	}

	AimedTargetActor = NewTarget;

	if (UAbilityReactionComponent* NewReaction = AimedTargetActor.IsValid()
		? AimedTargetActor->FindComponentByClass<UAbilityReactionComponent>()
		: nullptr)
	{
		NewReaction->SetAimedTarget(true);
	}
}

void FCryonisAbility::EnsurePreview()
{
	if (PlacementPreview.IsValid() || !Character || !Character->GetWorld())
	{
		return;
	}

	PlacementPreview = Character->GetWorld()->SpawnActor<AIcePlacementPreview>(FVector::ZeroVector, FRotator::ZeroRotator);
}

bool FCryonisAbility::TraceTarget(FHitResult& OutHit) const
{
	if (!Character || !Character->GetWorld() || !Character->GetController())
	{
		return false;
	}

	FVector TraceStart;
	FVector TraceEnd;

	if (bTargetAtFeet)
	{
		TraceStart = Character->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
		TraceEnd = Character->GetActorLocation() - FVector(0.0f, 0.0f, 200.0f);
	}
	else
	{
		TraceStart = Character->GetPawnViewLocation();
		TraceEnd = TraceStart + Character->GetController()->GetControlRotation().Vector() * TargetRange;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(IceTargetTrace), false, Character);
	QueryParams.AddIgnoredActor(Character);

	if (PlacementPreview.IsValid())
	{
		QueryParams.AddIgnoredActor(PlacementPreview.Get());
	}

	return Character->GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
}

bool FCryonisAbility::CanSpawnAt(const FVector& SpawnLocation, AActor* SurfaceActor) const
{
	if (!Character || !Character->GetWorld())
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(IceSpawnOverlap), false, Character);
	QueryParams.AddIgnoredActor(Character);

	if (SurfaceActor)
	{
		QueryParams.AddIgnoredActor(SurfaceActor);
	}

	if (PlacementPreview.IsValid())
	{
		QueryParams.AddIgnoredActor(PlacementPreview.Get());
	}

	FCollisionObjectQueryParams ObjectParams;

	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	TArray<FOverlapResult> Overlaps;

	const bool bHasOverlap = Character->GetWorld()->OverlapMultiByObjectType(Overlaps, SpawnLocation, FQuat::Identity, ObjectParams,FCollisionShape::MakeBox(FVector(75.0f, 75.0f, AIcePillar::HalfHeight)), QueryParams);

	return !bHasOverlap;
}

FVector FCryonisAbility::GetSpawnLocation(const FHitResult& Hit)
{
	return Hit.ImpactPoint + FVector(0.0f, 0.0f, AIcePillar::HalfHeight);
}

void FCryonisAbility::DestroyPillar(AIcePillar* Pillar)
{
	if (!IsValid(Pillar))
	{
		return;
	}

	Pillar->PlayDestroyEffect();
	Pillar->Destroy();
}
