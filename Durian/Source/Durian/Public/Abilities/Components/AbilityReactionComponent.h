#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "Abilities/Core/AbilityModeListener.h"
#include "Components/ActorComponent.h"
#include "AbilityReactionComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMeshComponent;

UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UAbilityReactionComponent : public UActorComponent, public IAbilityModeListener
{
	GENERATED_BODY()

public:
	UAbilityReactionComponent();

	EAbilityReactionType GetReactionType() const { return ReactionType; }

	void SetAimedTarget(bool bAimed);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual TArray<EAbilityVisualMode> GetSupportedAbilityModes_Implementation() const override;
	virtual void OnAbilityModeChanged_Implementation(EAbilityVisualMode Mode, bool bEnabled) override;

private:
	void SetTargetStencilEnabled(bool bEnabled) const;
	void SetTargetSurfaceHighlightEnabled(bool bEnabled, EAbilityVisualMode Mode);
	void CreateTargetSurfaceHighlightOverlays();

	UPROPERTY(EditAnywhere, Category = "Ability Reaction")
	EAbilityReactionType ReactionType = EAbilityReactionType::Normal;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Reaction|VFX")
	TSoftObjectPtr<UMaterialInterface> TargetSurfaceHighlightMaterial;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> TargetSurfaceHighlightOverlayArray;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> TargetSurfaceHighlightMaterialArray;

	bool bAimedTarget = false;
	EAbilityVisualMode ActiveMode = EAbilityVisualMode::None;
};
