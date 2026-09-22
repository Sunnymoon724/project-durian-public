#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/AbilityModeListener.h"
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
	void SetReactionType(const EAbilityReactionType NewReactionType) { ReactionType = NewReactionType; }
	void SetAimedTarget(bool bAimed);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual TArray<EAbilityMode> GetSupportedAbilityModes_Implementation() const override;
	virtual void OnAbilityModeChanged_Implementation(EAbilityMode Mode, bool bEnabled) override;

private:
	bool IsTargetReaction() const;
	void SetTargetStencilEnabled(bool bEnabled) const;
	void SetTopScanEnabled(bool bEnabled, EAbilityMode Mode);
	void SetTargetSurfaceHighlightEnabled(bool bEnabled, EAbilityMode Mode);
	void CreateTopScanOverlays();
	void CreateTargetSurfaceHighlightOverlays();

	UPROPERTY(EditAnywhere, Category = "Ability Reaction")
	EAbilityReactionType ReactionType = EAbilityReactionType::Normal;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Reaction|VFX")
	TSoftObjectPtr<UMaterialInterface> TopScanOverlayMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Reaction|VFX")
	TSoftObjectPtr<UMaterialInterface> TargetSurfaceHighlightMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> TopScanOverlays;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> TopScanMaterials;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> TargetSurfaceHighlightOverlays;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> TargetSurfaceHighlightMaterials;

	bool bAimedTarget = false;
	EAbilityMode ActiveMode = EAbilityMode::None;
};
