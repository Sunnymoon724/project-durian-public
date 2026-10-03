#include "Framework/Camera/CameraPostProcessComponent.h"

#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UCameraPostProcessComponent::UCameraPostProcessComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCameraPostProcessComponent::BeginPlay()
{
	Super::BeginPlay();

	CameraComponent = Cast<UCameraComponent>(GetAttachParent());

	if (!CameraComponent.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CameraPostProcessComponent must be attached to a CameraComponent on '%s'."), *GetNameSafe(GetOwner()));

		return;
	}

	for (FWeightedBlendable& Blendable : CameraComponent->PostProcessSettings.WeightedBlendables.Array)
	{
		UMaterialInterface* Material = Cast<UMaterialInterface>(Blendable.Object);

		if (!Material || EffectMaterialMap.Contains(Material->GetFName()))
		{
			continue;
		}

		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(Material, this);

		EffectMaterialMap.Add(Material->GetFName(), MaterialInstance);

		Blendable.Object = MaterialInstance;
	}
}

UMaterialInstanceDynamic* UCameraPostProcessComponent::GetEffectMaterial(const FName MaterialName) const
{
	const TObjectPtr<UMaterialInstanceDynamic>* Material = EffectMaterialMap.Find(MaterialName);

	return Material ? Material->Get() : nullptr;
}

void UCameraPostProcessComponent::EnableMaterial(const FName MaterialName, const TFunctionRef<void(UMaterialInstanceDynamic*)>& ConfigureMaterial) const
{
	if (!CameraComponent.IsValid())
	{
		return;
	}

	if (UMaterialInstanceDynamic* Material = GetEffectMaterial(MaterialName))
	{
		ConfigureMaterial(Material);

		CameraComponent->AddOrUpdateBlendable(Material, 1.0f);
	}
}

void UCameraPostProcessComponent::DisableMaterial(const FName MaterialName) const
{
	if (!CameraComponent.IsValid())
	{
		return;
	}

	if (UMaterialInstanceDynamic* Material = GetEffectMaterial(MaterialName))
	{
		CameraComponent->AddOrUpdateBlendable(Material, 0.0f);
	}
}
