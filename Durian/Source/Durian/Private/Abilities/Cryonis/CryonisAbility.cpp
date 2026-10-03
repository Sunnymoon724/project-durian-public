#include "Abilities/Cryonis/CryonisAbility.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "Abilities/Cryonis/IcePillar.h"
#include "Abilities/Cryonis/IcePlacementPreview.h"
#include "DataAssets/GameConstantsDataAsset.h"
#include "Enums/PlayerEnums.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Utilities/NiagaraEffectUtility.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/PrimitiveComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCryonisAbility::Tick(const float)
{
	if (!Character)
	{
		return;
	}

	if (Character->GetCurrentState() != EPlayerState::IceTargeting)
	{
		return;
	}

	if (Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling())
	{
		ExitTargetingMode();

		return;
	}

	UpdateTargeting();
}

float UCryonisAbility::GetSpawnCooldownRemaining() const
{
	return GetRemainingCooldown(SpawnCooldownEndTime);
}

void UCryonisAbility::HandleInput(const EAbilityInput Input, float)
{
	if (!Character)
	{
		return;
	}

	switch (Input)
	{
	case EAbilityInput::Interact:
		{
			if (Character->GetCurrentState() != EPlayerState::IceTargeting || !Character->IsScanPoseReady())
			{
				return;
			}
			if (TargetPillar.IsValid())
			{
				RemoveTargetedPillar();
			}
			else if (bTargetValid && GetSpawnCooldownRemaining() <= 0.0f)
			{
				SpawnIcePillar();
			}
			break;
		}
	case EAbilityInput::Cancel:
	case EAbilityInput::Interrupt:
		{
			if (Character->GetCurrentState() == EPlayerState::IceTargeting)
			{
				ExitTargetingMode();
			}
			break;
		}
	case EAbilityInput::Use:
		{
			const EPlayerState PreviousState = Character->GetCurrentState();

			if (PreviousState == EPlayerState::Normal)
			{
				Character->SetPlayerState(EPlayerState::IceTargeting);
				bTargetAtFeet = false;
			}
			else if (PreviousState == EPlayerState::IceTargeting)
			{
				ExitTargetingMode();
			}
		}
		break;
	case EAbilityInput::CryonisTargetAtFeet:
		{
			if (Character->GetCurrentState() == EPlayerState::IceTargeting)
			{
				bTargetAtFeet = !bTargetAtFeet;
			}
			break;
		}
	default:
		break;
	}
}

bool UCryonisAbility::IsIcePillarTarget(const AActor* Actor)
{
	return Actor && (Actor->ActorHasTag(TEXT("IcePillar")) || Actor->IsA<AIcePillar>());
}

void UCryonisAbility::UpdateTargeting()
{
	if (!UpdateScanActivation(EAbilityVisualMode::Cryonis, EAbilityType::Cryonis, true))
	{
		ClearTarget();
		if (PlacementPreview.IsValid()) PlacementPreview->SetPreviewState(FVector::ZeroVector, false, false);
		return;
	}
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

	if (!bTargetAtFeet && HitActor && !IsIcePillarTarget(HitActor) && Character->GetCapsuleComponent())
	{
		const FVector CandidateLocation = GetSpawnLocation(Hit);
		const float PillarHalfHeight = UGameConstantsDataAsset::Get()->IcePillarHeight * 0.5f;
		const FBox CandidateBounds(CandidateLocation - FVector(75.0f, 75.0f, PillarHalfHeight), CandidateLocation + FVector(75.0f, 75.0f, PillarHalfHeight));

		if (CandidateBounds.Intersect(Character->GetCapsuleComponent()->Bounds.GetBox()))
		{
			FHitResult FootHit;

			if (!TraceAbilityTarget(EAbilityReactionType::CryonicTarget, UGameConstantsDataAsset::Get()->CryonisTargetRange, FootHit, true, PlacementPreview.Get()) || IsIcePillarTarget(FootHit.GetActor()))
			{
				UpdateAimedTarget(nullptr);
				PlacementPreview->SetPreviewState(FVector::ZeroVector, false, false);

				return;
			}

			Hit = FootHit;
			HitActor = FootHit.GetActor();
		}
	}

	if (IsIcePillarTarget(HitActor))
	{
		UpdateAimedTarget(HitActor);

		TargetPillar = Cast<AIcePillar>(HitActor);
		PlacementPreview->SetPreviewState(FVector::ZeroVector, false, false);

		return;
	}

	if (!HitActor)
	{
		UpdateAimedTarget(nullptr);

		PlacementPreview->SetPreviewState(FVector::ZeroVector, false, false);

		return;
	}

	TargetSurface = HitActor;
	UpdateAimedTarget(HitActor);

	TargetLocation = GetSpawnLocation(Hit);
	bTargetValid = GetSpawnCooldownRemaining() <= 0.0f && CanSpawnAt(TargetLocation, HitActor);
	PlacementPreview->SetPreviewState(TargetLocation, true, bTargetValid);
}

void UCryonisAbility::SpawnIcePillar()
{
	if (!Character || !Character->GetWorld() || !TargetSurface.IsValid() || !bTargetValid)
	{
		return;
	}

	const FVector SpawnLocation = PlacementPreview.IsValid() ? PlacementPreview->GetActorLocation() : TargetLocation;

	if (!CanSpawnAt(SpawnLocation, TargetSurface.Get()))
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;

	SpawnParameters.Owner = Character;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* PillarClass = LoadClass<AIcePillar>(nullptr, TEXT("/Game/_BP/Abilities/BP_IcePillar.BP_IcePillar_C"));

	if (!PillarClass)
	{
		return;
	}

	AIcePillar* NewPillar = Character->GetWorld()->SpawnActor<AIcePillar>(PillarClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);

	if (!NewPillar)
	{
		return;
	}

	SpawnedPillars.Add(NewPillar);
	SpawnCooldownEndTime = Character->GetWorld()->GetTimeSeconds() + UGameConstantsDataAsset::Get()->IcePillarSpawnCooldown;
	FNiagaraEffectUtility::SpawnAtLocation(Character->GetWorld(), FSoftObjectPath(TEXT("/Game/Resources/VFX/Cryonis/Niagara/NS_IceSpawnSplash.NS_IceSpawnSplash")), SpawnLocation);

	while (SpawnedPillars.Num() > UGameConstantsDataAsset::Get()->MaxIcePillarCount)
	{
		if (AIcePillar* OldestPillar = SpawnedPillars[0].Get())
		{
			DestroyPillar(OldestPillar);
		}

		SpawnedPillars.RemoveAt(0);
	}
}

void UCryonisAbility::RemoveTargetedPillar()
{
	AIcePillar* Pillar = TargetPillar.Get();

	if (!Pillar)
	{
		return;
	}

	DestroyPillar(Pillar, true);
	SpawnedPillars.Remove(Pillar);

	ClearTarget();
}

void UCryonisAbility::ExitTargetingMode()
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

	SetAbilityModeActive(EAbilityVisualMode::Cryonis, false);
	ClearAbilityVisionEffects();
}

void UCryonisAbility::ClearTarget()
{
	UpdateAimedTarget(nullptr);

	TargetSurface.Reset();
	TargetPillar.Reset();
	TargetLocation = FVector::ZeroVector;
	bTargetValid = false;
}

void UCryonisAbility::UpdateAimedTarget(AActor* NewTarget)
{
	if (AimedTargetActor.Get() == NewTarget)
	{
		return;
	}

	if (UAbilityReactionComponent* PreviousReaction = AimedTargetActor.IsValid() ? AimedTargetActor->FindComponentByClass<UAbilityReactionComponent>() : nullptr)
	{
		PreviousReaction->SetAimedTarget(false);
	}

	AimedTargetActor = NewTarget;

	if (UAbilityReactionComponent* NewReaction = AimedTargetActor.IsValid() ? AimedTargetActor->FindComponentByClass<UAbilityReactionComponent>() : nullptr)
	{
		NewReaction->SetAimedTarget(true);
	}
}

void UCryonisAbility::EnsurePreview()
{
	if (PlacementPreview.IsValid() || !Character || !Character->GetWorld())
	{
		return;
	}

	static UClass* PreviewClass = LoadClass<AIcePlacementPreview>(nullptr, TEXT("/Game/_BP/Abilities/BP_IcePlacementPreview.BP_IcePlacementPreview_C"));
	PlacementPreview = Character->GetWorld()->SpawnActor<AIcePlacementPreview>(PreviewClass ? PreviewClass : AIcePlacementPreview::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
}

bool UCryonisAbility::TraceTarget(FHitResult& OutHit) const
{
	return TraceAbilityTarget(EAbilityReactionType::CryonicTarget, UGameConstantsDataAsset::Get()->CryonisTargetRange, OutHit, bTargetAtFeet, PlacementPreview.Get());
}

bool UCryonisAbility::CanSpawnAt(const FVector& SpawnLocation, const AActor* SurfaceActor) const
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

	const bool bHasOverlap = Character->GetWorld()->OverlapMultiByObjectType(Overlaps, SpawnLocation, FQuat::Identity, ObjectParams, FCollisionShape::MakeBox(FVector(75.0f, 75.0f, UGameConstantsDataAsset::Get()->IcePillarHeight * 0.5f)), QueryParams);

	return !bHasOverlap;
}

FVector UCryonisAbility::GetSpawnLocation(const FHitResult& Hit)
{
	return Hit.ImpactPoint + FVector(0.0f, 0.0f, UGameConstantsDataAsset::Get()->IcePillarHeight * 0.5f);
}

void UCryonisAbility::DestroyPillar(AIcePillar* Pillar, const bool bShatter)
{
	if (!IsValid(Pillar))
	{
		return;
	}

	Pillar->PlayDestroyEffect(bShatter);
	Pillar->Destroy();
}
