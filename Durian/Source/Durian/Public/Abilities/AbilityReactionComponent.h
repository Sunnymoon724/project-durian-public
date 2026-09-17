#pragma once

#include "CoreMinimal.h"
#include "Abilities/AbilityModeListener.h"
#include "Components/ActorComponent.h"
#include "AbilityReactionComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;

/** 월드 오브젝트 하나의 능력 모드 반응을 담당한다. */
UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UAbilityReactionComponent : public UActorComponent, public IAbilityModeListener
{
	GENERATED_BODY()

public:
	UAbilityReactionComponent();
	EAbilityReactionType GetReactionType() const { return ReactionType; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual TArray<EAbilityMode> GetSupportedAbilityModes_Implementation() const override;
	virtual void OnAbilityModeChanged_Implementation(EAbilityMode Mode, bool bEnabled) override;

private:
	void SetMagnetStencilEnabled(bool bEnabled) const;
	void SetTopScanEnabled(bool bEnabled, EAbilityMode Mode);
	void CreateTopScanOverlays();
	FLinearColor GetModeColor(EAbilityMode Mode) const;

	UPROPERTY(EditAnywhere, Category = "Ability Reaction")
	EAbilityReactionType ReactionType = EAbilityReactionType::Normal;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Reaction|VFX")
	TSoftObjectPtr<UMaterialInterface> TopScanOverlayMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> TopScanOverlays;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> TopScanMaterials;
};
