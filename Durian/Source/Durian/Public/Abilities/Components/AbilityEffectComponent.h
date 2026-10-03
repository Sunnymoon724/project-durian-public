#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enums/PlayerEnums.h"
#include "AbilityEffectComponent.generated.h"

class UCameraComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UNiagaraSystem;

/**
 * Controls post-process materials configured on the player's camera.
 * Ability vision is one consumer; other camera effects can use SetEffectWeight.
 */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UAbilityEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityEffectComponent();

	void SetVisionEnabled(const EAbilityType Ability, const bool bEnabled);
	void ClearVisionEffects();
	void PlayEnterPulse(EAbilityType Ability);
	void SetEffectWeight(FName MaterialName, float Weight);

	virtual void BeginPlay() override;

private:
	void ApplyVisionProfile(UMaterialInstanceDynamic* MaterialInstance, EAbilityType Ability) const;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UMaterialInstanceDynamic>> EffectMaterials;

	UPROPERTY(EditDefaultsOnly, Category = "Ability VFX|Pulse")
	TMap<EAbilityType, TSoftObjectPtr<UNiagaraSystem>> EnterPulseSystems;

	TWeakObjectPtr<UCameraComponent> CameraComponent;
};
