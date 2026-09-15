#include "Abilities/AbilityVFXComponent.h"

#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UAbilityVFXComponent::UAbilityVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	VisionMaterials.Add(
		EAbilityType::Magnet,
		TSoftObjectPtr<UMaterialInterface>(
			FSoftObjectPath(TEXT("/Game/Resources/VFX/Magnet/PP/MI_PP_MagnetVision.MI_PP_MagnetVision"))));
}

void UAbilityVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	CameraComponent = GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;
	if (!CameraComponent.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityVFXComponent: Owner '%s' has no CameraComponent."), *GetNameSafe(GetOwner()));
		return;
	}

	for (const TPair<EAbilityType, TSoftObjectPtr<UMaterialInterface>>& VisionMaterial : VisionMaterials)
	{
		if (UMaterialInstanceDynamic* MaterialInstance = GetOrCreateVisionMaterial(VisionMaterial.Key))
		{
			SetBlendableWeight(MaterialInstance, 0.0f);
		}
	}
}

void UAbilityVFXComponent::SetVisionEnabled(EAbilityType Ability, bool bEnabled)
{
	if (!CameraComponent.IsValid())
	{
		return;
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
	}

	if (UMaterialInstanceDynamic* MaterialInstance = GetOrCreateVisionMaterial(Ability))
	{
		SetBlendableWeight(MaterialInstance, bEnabled ? 1.0f : 0.0f);
	}
}

void UAbilityVFXComponent::ClearVisionEffects()
{
	for (const TPair<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>>& VisionMaterial : VisionMaterialInstances)
	{
		SetBlendableWeight(VisionMaterial.Value, 0.0f);
	}
}

UMaterialInstanceDynamic* UAbilityVFXComponent::GetOrCreateVisionMaterial(EAbilityType Ability)
{
	if (const TObjectPtr<UMaterialInstanceDynamic>* ExistingMaterial = VisionMaterialInstances.Find(Ability))
	{
		return *ExistingMaterial;
	}

	const TSoftObjectPtr<UMaterialInterface>* MaterialAsset = VisionMaterials.Find(Ability);
	if (!MaterialAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityVFXComponent: No vision material configured for ability %d."), static_cast<int32>(Ability));
		return nullptr;
	}

	UMaterialInterface* ParentMaterial = MaterialAsset->LoadSynchronous();
	if (!ParentMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityVFXComponent: Could not load vision material for ability %d."), static_cast<int32>(Ability));
		return nullptr;
	}

	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(ParentMaterial, this);
	VisionMaterialInstances.Add(Ability, MaterialInstance);
	return MaterialInstance;
}

void UAbilityVFXComponent::SetBlendableWeight(UMaterialInstanceDynamic* MaterialInstance, float Weight)
{
	if (CameraComponent.IsValid() && MaterialInstance)
	{
		CameraComponent->AddOrUpdateBlendable(MaterialInstance, Weight);
	}
}
