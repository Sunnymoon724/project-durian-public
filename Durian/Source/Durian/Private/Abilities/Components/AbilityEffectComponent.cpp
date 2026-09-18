#include "Abilities/Components/AbilityEffectComponent.h"

#include "Abilities/Core/AbilityModeVisualProfile.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UAbilityEffectComponent::UAbilityEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// All vision instances share M_PP_AbilityModeWorldScan. Each ability instance
	// supplies only its own tint and target-emphasis parameters.
	VisionMaterials.Add(EAbilityType::Magnesis, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Resources/VFX/Magnesis/PP/MI_PP_MagnesisWorldScan.MI_PP_MagnesisWorldScan"))));
	VisionMaterials.Add(EAbilityType::Cryonis, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Resources/VFX/AbilityMode/PP/MI_PP_IceWorldScan.MI_PP_IceWorldScan"))));
	VisionMaterials.Add(EAbilityType::Stasis, TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Resources/VFX/AbilityMode/PP/MI_PP_TimeLockWorldScan.MI_PP_TimeLockWorldScan"))));
	MagnesisHoldLinkSystem = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/Resources/VFX/Magnesis/Niagara/NS_MagnesisHoldLink.NS_MagnesisHoldLink")));
}

void UAbilityEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	CameraComponent = GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;
	PlayerCameraManager = GetWorld() ? UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0) : nullptr;

	if (!CameraComponent.IsValid() && !PlayerCameraManager.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityEffectComponent: No camera or PlayerCameraManager for '%s'."), *GetNameSafe(GetOwner()));

		return;
	}

	for (const TPair<EAbilityType, TSoftObjectPtr<UMaterialInterface>>& VisionMaterial : VisionMaterials)
	{
		if (UMaterialInstanceDynamic* MaterialInstance = GetOrCreateVisionMaterial(VisionMaterial.Key))
		{
			VisionWeights.Add(VisionMaterial.Key, 0.0f);
			SetBlendableWeight(MaterialInstance, 0.0f);
		}
	}

	for (const TPair<EAbilityType, TSoftObjectPtr<UMaterialInterface>>& HighlightMaterial : HighlightMaterials)
	{
		if (UMaterialInstanceDynamic* MaterialInstance = GetOrCreateHighlightMaterial(HighlightMaterial.Key))
		{
			HighlightWeights.Add(HighlightMaterial.Key, 0.0f);
			SetBlendableWeight(MaterialInstance, 0.0f);
		}
	}

}

void UAbilityEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CameraComponent.IsValid() && !PlayerCameraManager.IsValid() && GetWorld())
	{
		PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	}

	ApplyCameraManagerBlendables();
}

void UAbilityEffectComponent::SetVisionEnabled(const EAbilityType Ability, const bool bEnabled)
{
	if (!CameraComponent.IsValid())
	{
		if (!PlayerCameraManager.IsValid())
		{
			return;
		}
	}

	if (bEnabled)
	{
		for (const TPair<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>>& VisionMaterial : VisionMaterialInstances)
		{
			if (VisionMaterial.Key != Ability)
			{
				SetBlendableWeight(VisionMaterial.Value, 0.0f);
			}
		}

		for (const TPair<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>>& HighlightMaterial : HighlightMaterialInstances)
		{
			if (HighlightMaterial.Key != Ability)
			{
				SetBlendableWeight(HighlightMaterial.Value, 0.0f);
			}
		}
	}

	if (UMaterialInstanceDynamic* MaterialInstance = GetOrCreateVisionMaterial(Ability))
	{
		ApplyVisionProfile(MaterialInstance, Ability);
		VisionWeights.Add(Ability, bEnabled ? 1.0f : 0.0f);
		SetBlendableWeight(MaterialInstance, bEnabled ? 1.0f : 0.0f);
	}

	if (UMaterialInstanceDynamic* MaterialInstance = GetOrCreateHighlightMaterial(Ability))
	{
		HighlightWeights.Add(Ability, bEnabled ? 1.0f : 0.0f);
		SetBlendableWeight(MaterialInstance, bEnabled ? 1.0f : 0.0f);
	}

}

void UAbilityEffectComponent::ApplyVisionProfile(UMaterialInstanceDynamic* MaterialInstance, const EAbilityType Ability) const
{
	if (!MaterialInstance)
	{
		return;
	}

	EAbilityMode Mode = EAbilityMode::None;
	switch (Ability)
	{
	case EAbilityType::Magnesis: Mode = EAbilityMode::Magnesis; break;
	case EAbilityType::Cryonis: Mode = EAbilityMode::Cryonis; break;
	case EAbilityType::Stasis: Mode = EAbilityMode::Stasis; break;
	case EAbilityType::RemoteBomb: Mode = EAbilityMode::RemoteBomb; break;
	case EAbilityType::None:
	default: return;
	}

	const FAbilityModeVisualProfile& Profile = FAbilityModeVisualProfiles::Get(Mode);
	const FAbilityModeVisualCommonProfile& Common = FAbilityModeVisualProfiles::GetCommon();
	MaterialInstance->SetVectorParameterValue(TEXT("WorldGradeColor"), Profile.WorldGradeColor);
	MaterialInstance->SetScalarParameterValue(TEXT("WorldBlend"), Common.WorldBlend);
	MaterialInstance->SetVectorParameterValue(TEXT("CandidateGradeColor"), Profile.CandidateGradeColor);
	MaterialInstance->SetVectorParameterValue(TEXT("CandidateGlowColor"), Profile.CandidateGlowColor);
	MaterialInstance->SetScalarParameterValue(TEXT("CandidateBlend"), Common.CandidateBlend);
	MaterialInstance->SetVectorParameterValue(TEXT("TargetGradeColor"), Profile.TargetGradeColor);
	MaterialInstance->SetVectorParameterValue(TEXT("TargetGlowColor"), Profile.TargetGlowColor);
	MaterialInstance->SetScalarParameterValue(TEXT("TargetBlend"), Common.TargetBlend);
}

void UAbilityEffectComponent::ClearVisionEffects()
{
	for (const TPair<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>>& VisionMaterial : VisionMaterialInstances)
	{
		SetBlendableWeight(VisionMaterial.Value, 0.0f);
		VisionWeights.Add(VisionMaterial.Key, 0.0f);
	}


	for (const TPair<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>>& HighlightMaterial : HighlightMaterialInstances)
	{
		SetBlendableWeight(HighlightMaterial.Value, 0.0f);
		HighlightWeights.Add(HighlightMaterial.Key, 0.0f);
	}
}

void UAbilityEffectComponent::PlayEnterPulse(EAbilityType Ability)
{
	const TSoftObjectPtr<UNiagaraSystem>* PulseAsset = EnterPulseSystems.Find(Ability);

	if (!PulseAsset || !GetOwner())
	{
		return;
	}

	UNiagaraSystem* PulseSystem = PulseAsset->LoadSynchronous();

	if (!PulseSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityEffectComponent: Could not load enter pulse for ability %d."), static_cast<int32>(Ability));
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),PulseSystem,GetOwner()->GetActorLocation(),GetOwner()->GetActorRotation(),FVector::OneVector,true,true,ENCPoolMethod::AutoRelease,true);
}

void UAbilityEffectComponent::UpdateMagnesisHoldLink(UPrimitiveComponent* TargetComponent, const FVector& StartLocation, const FVector& EndLocation)
{
	if (!IsValid(TargetComponent) || !GetWorld())
	{
		ClearMagnesisHoldLink();
		return;
	}

	if (!IsValid(MagnesisHoldLinkComponent))
	{
		UNiagaraSystem* LinkSystem = MagnesisHoldLinkSystem.LoadSynchronous();
		if (!LinkSystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("AbilityEffectComponent: Could not load Magnesis hold link system."));
			return;
		}

		MagnesisHoldLinkComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LinkSystem, StartLocation, FRotator::ZeroRotator, FVector::OneVector, false, true, ENCPoolMethod::None, true);
	}

	if (IsValid(MagnesisHoldLinkComponent))
	{
		MagnesisHoldLinkComponent->SetVariablePosition(TEXT("User.BeamStart"), StartLocation);
		MagnesisHoldLinkComponent->SetVariablePosition(TEXT("User.BeamEnd"), EndLocation);
		MagnesisHoldLinkComponent->Activate(true);
	}
}

void UAbilityEffectComponent::ClearMagnesisHoldLink()
{
	if (IsValid(MagnesisHoldLinkComponent))
	{
		MagnesisHoldLinkComponent->DeactivateImmediate();
		MagnesisHoldLinkComponent->DestroyComponent();
	}

	MagnesisHoldLinkComponent = nullptr;
}

UMaterialInstanceDynamic* UAbilityEffectComponent::GetOrCreateVisionMaterial(EAbilityType Ability)
{
	if (const TObjectPtr<UMaterialInstanceDynamic>* ExistingMaterial = VisionMaterialInstances.Find(Ability))
	{
		return *ExistingMaterial;
	}

	const TSoftObjectPtr<UMaterialInterface>* MaterialAsset = VisionMaterials.Find(Ability);

	if (!MaterialAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityEffectComponent: No vision material configured for ability %d."), static_cast<int32>(Ability));

		return nullptr;
	}

	UMaterialInterface* ParentMaterial = MaterialAsset->LoadSynchronous();

	if (!ParentMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityEffectComponent: Could not load vision material for ability %d."), static_cast<int32>(Ability));

		return nullptr;
	}

	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(ParentMaterial, this);
	VisionMaterialInstances.Add(Ability, MaterialInstance);

	return MaterialInstance;
}

UMaterialInstanceDynamic* UAbilityEffectComponent::GetOrCreateHighlightMaterial(EAbilityType Ability)
{
	if (const TObjectPtr<UMaterialInstanceDynamic>* ExistingMaterial = HighlightMaterialInstances.Find(Ability))
	{
		return *ExistingMaterial;
	}

	const TSoftObjectPtr<UMaterialInterface>* MaterialAsset = HighlightMaterials.Find(Ability);
	if (!MaterialAsset)
	{
		return nullptr;
	}

	UMaterialInterface* ParentMaterial = MaterialAsset->LoadSynchronous();
	if (!ParentMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityEffectComponent: Could not load highlight material for ability %d."), static_cast<int32>(Ability));
		return nullptr;
	}

	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(ParentMaterial, this);
	HighlightMaterialInstances.Add(Ability, MaterialInstance);
	return MaterialInstance;
}

void UAbilityEffectComponent::SetBlendableWeight(UMaterialInstanceDynamic* MaterialInstance, const float Weight) const
{
	if (CameraComponent.IsValid() && MaterialInstance)
	{
		CameraComponent->AddOrUpdateBlendable(MaterialInstance, Weight);
	}
}

void UAbilityEffectComponent::ApplyCameraManagerBlendables()
{
	if (CameraComponent.IsValid() || !PlayerCameraManager.IsValid())
	{
		return;
	}

	for (const TPair<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>>& VisionMaterial : VisionMaterialInstances)
	{
		if (const float* Weight = VisionWeights.Find(VisionMaterial.Key); Weight && *Weight > 0.0f)
		{
			FPostProcessSettings Settings;
			Settings.AddBlendable(VisionMaterial.Value, *Weight);
			PlayerCameraManager->AddCachedPPBlend(Settings, 1.0f);
		}
	}

	for (const TPair<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>>& HighlightMaterial : HighlightMaterialInstances)
	{
		if (const float* Weight = HighlightWeights.Find(HighlightMaterial.Key); Weight && *Weight > 0.0f)
		{
			FPostProcessSettings Settings;
			Settings.AddBlendable(HighlightMaterial.Value, *Weight);
			PlayerCameraManager->AddCachedPPBlend(Settings, 1.0f);
		}
	}
}
