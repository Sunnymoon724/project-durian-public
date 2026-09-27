#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IcePillar.generated.h"

class UAbilityReactionComponent;
class USceneComponent;
class UStaticMeshComponent;
class UNiagaraSystem;

UCLASS()
class DURIAN_API AIcePillar : public AActor
{
	GENERATED_BODY()

public:
	AIcePillar();
	virtual void Tick(float DeltaSeconds) override;

	void PlayDestroyEffect(bool bShatter) const;
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSoftObjectPtr<UNiagaraSystem> ShatterEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSoftObjectPtr<UNiagaraSystem> DissolveEffect;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> IcePillar;

	float SpawnAnimationElapsed = 0.0f;

	float PillarHeight = 1.0f;
	float PillarHorizontalScale = 1.0f;
	float PillarAnimationDuration = 1.0f;
	bool IsSetupValid() const;
};
