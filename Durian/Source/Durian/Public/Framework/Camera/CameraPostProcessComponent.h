#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Templates/Function.h"
#include "CameraPostProcessComponent.generated.h"

class UCameraComponent;
class UMaterialInstanceDynamic;

/** Controls post-process materials configured on the parent camera. */
UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class DURIAN_API UCameraPostProcessComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCameraPostProcessComponent();

	void EnableMaterial(const FName MaterialName, const TFunctionRef<void(UMaterialInstanceDynamic*)>& ConfigureMaterial) const;
	void DisableMaterial(const FName MaterialName) const;

	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UMaterialInstanceDynamic>> EffectMaterialMap;

	UMaterialInstanceDynamic* GetEffectMaterial(FName MaterialName) const;

	TWeakObjectPtr<UCameraComponent> CameraComponent;
};
