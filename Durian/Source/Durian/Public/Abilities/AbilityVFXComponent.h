#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enums/PlayerEnums.h"
#include "AbilityVFXComponent.generated.h"

class UCameraComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

/**
 * 플레이어 능력의 화면 효과를 공통으로 관리한다.
 * 각 능력은 이 컴포넌트에 상태만 전달하고, 포스트 프로세스 연결은 여기서 처리한다.
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UAbilityVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityVFXComponent();

	/** 지정한 능력의 화면 효과를 켜거나 끈다. 동시에 하나의 능력 시야만 활성화한다. */
	UFUNCTION(BlueprintCallable, Category = "Ability VFX")
	void SetVisionEnabled(EAbilityType Ability, bool bEnabled);

	/** 현재 활성화된 모든 능력 화면 효과를 끈다. */
	UFUNCTION(BlueprintCallable, Category = "Ability VFX")
	void ClearVisionEffects();

protected:
	virtual void BeginPlay() override;

private:
	UMaterialInstanceDynamic* GetOrCreateVisionMaterial(EAbilityType Ability);
	void SetBlendableWeight(UMaterialInstanceDynamic* MaterialInstance, float Weight);

	UPROPERTY(EditDefaultsOnly, Category = "Ability VFX|Vision")
	TMap<EAbilityType, TSoftObjectPtr<UMaterialInterface>> VisionMaterials;

	UPROPERTY(Transient)
	TMap<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>> VisionMaterialInstances;

	TWeakObjectPtr<UCameraComponent> CameraComponent;
};
