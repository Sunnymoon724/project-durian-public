#include "Abilities/Components/AbilityEffectComponent.h"

#include "Abilities/Core/AbilityModeSubsystem.h"
#include "DataAssets/AbilityModeVisualProfile.h"
#include "Abilities/Core/AbilityVisualModeUtility.h"
#include "Utilities/NiagaraEffectUtility.h"
#include "Camera/CameraComponent.h"
#include "Engine/GameInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"

namespace
{
	const FName AbilityVisionMaterialName(TEXT("M_PP_AbilityModeWorldScan"));
}

UAbilityEffectComponent::UAbilityEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAbilityEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	CameraComponent = GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!CameraComponent.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityEffectComponent: No camera for '%s'."), *GetNameSafe(GetOwner()));

		return;
	}

	for (FWeightedBlendable& Blendable : CameraComponent->PostProcessSettings.WeightedBlendables.Array)
	{
		UMaterialInterface* Material = Cast<UMaterialInterface>(Blendable.Object);
		if (!Material || EffectMaterials.Contains(Material->GetFName()))
		{
			continue;
		}

		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
		EffectMaterials.Add(Material->GetFName(), MaterialInstance);
		Blendable.Object = MaterialInstance;
	}
}

void UAbilityEffectComponent::SetVisionEnabled(const EAbilityType Ability, const bool bEnabled)
{
	const TObjectPtr<UMaterialInstanceDynamic>* MaterialInstance = EffectMaterials.Find(AbilityVisionMaterialName);
	if (!MaterialInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityEffectComponent: Camera is missing %s."), *AbilityVisionMaterialName.ToString());
		return;
	}

	if (bEnabled)
	{
		ApplyVisionProfile(*MaterialInstance, Ability);
	}

	SetEffectWeight(AbilityVisionMaterialName, bEnabled ? 1.0f : 0.0f);
}

void UAbilityEffectComponent::ApplyVisionProfile(UMaterialInstanceDynamic* MaterialInstance, const EAbilityType Ability) const
{
	if (!MaterialInstance)
	{
		return;
	}

	const EAbilityVisualMode Mode = FAbilityVisualModeUtility::FromAbilityType(Ability);
	if (Mode == EAbilityVisualMode::None)
	{
		return;
	}

	const FAbilityModeVisualProfile& Profile = FAbilityModeVisualProfiles::Get(Mode);
	const FAbilityModeVisualCommonProfile& Common = FAbilityModeVisualProfiles::GetCommon();

	MaterialInstance->SetVectorParameterValue(TEXT("WorldGradeColor"), Profile.WorldGradeColor);
	MaterialInstance->SetVectorParameterValue(TEXT("ScanColor"), Profile.ScanColor);
	MaterialInstance->SetVectorParameterValue(TEXT("CandidateGradeColor"), Profile.CandidateGradeColor);
	MaterialInstance->SetVectorParameterValue(TEXT("CandidateGlowColor"), Profile.CandidateGlowColor);
	MaterialInstance->SetVectorParameterValue(TEXT("TargetGradeColor"), Profile.TargetScanColor);
	MaterialInstance->SetVectorParameterValue(TEXT("TargetGlowColor"), Profile.TargetEdgeColor);
	MaterialInstance->SetScalarParameterValue(TEXT("WorldBlend"), Common.WorldBlend);
	MaterialInstance->SetScalarParameterValue(TEXT("CandidateBlend"), Common.CandidateBlend);
	MaterialInstance->SetScalarParameterValue(TEXT("TargetBlend"), Common.TargetBlend);

	FVector2D ScanDirection(1.0f, 0.0f);
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (GameInstance)
	{
		if (const UAbilityModeSubsystem* AbilityModeSubsystem = GameInstance->GetSubsystem<UAbilityModeSubsystem>())
		{
			ScanDirection = AbilityModeSubsystem->GetModeScanDirection();
		}
	}

	MaterialInstance->SetScalarParameterValue(TEXT("ScanAngle"), FMath::Atan2(ScanDirection.Y, ScanDirection.X));
}

void UAbilityEffectComponent::ClearVisionEffects()
{
	SetEffectWeight(AbilityVisionMaterialName, 0.0f);
}
void UAbilityEffectComponent::PlayEnterPulse(EAbilityType Ability)
{
	const TSoftObjectPtr<UNiagaraSystem>* PulseAsset = EnterPulseSystems.Find(Ability);

	if (!PulseAsset || !GetOwner())
	{
		return;
	}

	FNiagaraEffectUtility::SpawnAtLocation(GetWorld(), *PulseAsset, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation());
}

void UAbilityEffectComponent::SetEffectWeight(const FName MaterialName, const float Weight)
{
	if (CameraComponent.IsValid())
	{
		if (const TObjectPtr<UMaterialInstanceDynamic>* Material = EffectMaterials.Find(MaterialName))
		{
			CameraComponent->AddOrUpdateBlendable(*Material, FMath::Clamp(Weight, 0.0f, 1.0f));
		}
	}
}

