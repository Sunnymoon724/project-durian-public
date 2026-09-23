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

	static constexpr float HalfHeight = 100.0f;
	static constexpr float HorizontalScale = 1.5f;
	static constexpr float FullHeightScale = 2.0f;

	void PlayDestroyEffect();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ice")
	TObjectPtr<UStaticMeshComponent> IceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ice")
	TObjectPtr<UAbilityReactionComponent> ReactionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Ice|Animation")
	float SpawnAnimationDuration = 0.45f;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	float SpawnAnimationElapsed = 0.0f;
};
