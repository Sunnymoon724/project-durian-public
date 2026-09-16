#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enums/PlayerEnums.h"
#include "AbilityEffectComponent.generated.h"

class UCameraComponent;
class APlayerCameraManager;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class UPrimitiveComponent;

/**
 * 플레이어 능력의 화면 효과를 공통으로 관리한다.
 * 각 능력은 이 컴포넌트에 상태만 전달하고, 포스트 프로세스 연결은 여기서 처리한다.
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UAbilityEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityEffectComponent();

	/** 지정한 능력의 화면 효과를 켜거나 끈다. 동시에 하나의 능력 시야만 활성화한다. */
	UFUNCTION(BlueprintCallable, Category = "Ability VFX")
	void SetVisionEnabled(const EAbilityType Ability, const bool bEnabled);

	/** 현재 활성화된 모든 능력 화면 효과를 끈다. */
	UFUNCTION(BlueprintCallable, Category = "Ability VFX")
	void ClearVisionEffects();

	/** 능력 진입 시에만 재생하는 1회성 월드 이펙트다. */
	UFUNCTION(BlueprintCallable, Category = "Ability VFX")
	void PlayEnterPulse(EAbilityType Ability);

	/** 잡은 자력 대상까지의 연결선을 생성하거나 위치를 갱신한다. */
	void UpdateMagnetHoldLink(UPrimitiveComponent* TargetComponent, const FVector& StartLocation, const FVector& EndLocation);

	/** 현재 자력 연결선을 즉시 정리한다. */
	void ClearMagnetHoldLink();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UMaterialInstanceDynamic* GetOrCreateVisionMaterial(EAbilityType Ability);
	UMaterialInstanceDynamic* GetOrCreateHighlightMaterial(EAbilityType Ability);
	void SetBlendableWeight(UMaterialInstanceDynamic* MaterialInstance, const float Weight) const;
	void ApplyCameraManagerBlendables();

	UPROPERTY(EditDefaultsOnly, Category = "Ability VFX|Vision")
	TMap<EAbilityType, TSoftObjectPtr<UMaterialInterface>> VisionMaterials;

	UPROPERTY(Transient)
	TMap<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>> VisionMaterialInstances;

	UPROPERTY(EditDefaultsOnly, Category = "Ability VFX|Highlight")
	TMap<EAbilityType, TSoftObjectPtr<UMaterialInterface>> HighlightMaterials;

	UPROPERTY(Transient)
	TMap<EAbilityType, TObjectPtr<UMaterialInstanceDynamic>> HighlightMaterialInstances;

	UPROPERTY(EditDefaultsOnly, Category = "Ability VFX|Pulse")
	TMap<EAbilityType, TSoftObjectPtr<UNiagaraSystem>> EnterPulseSystems;

	UPROPERTY(EditDefaultsOnly, Category = "Ability VFX|Magnet")
	TSoftObjectPtr<UNiagaraSystem> MagnetHoldLinkSystem;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> MagnetHoldLinkComponent;

	TWeakObjectPtr<UCameraComponent> CameraComponent;
	TWeakObjectPtr<APlayerCameraManager> PlayerCameraManager;

	TMap<EAbilityType, float> VisionWeights;
	TMap<EAbilityType, float> HighlightWeights;
};
