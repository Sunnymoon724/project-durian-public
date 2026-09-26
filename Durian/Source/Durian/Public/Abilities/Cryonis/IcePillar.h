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
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	static constexpr float HorizontalScale = 1.5f;

	void PlayDestroyEffect(bool bShatter) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ice")
	TObjectPtr<UStaticMeshComponent> IceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ice")
	TObjectPtr<UAbilityReactionComponent> ReactionComponent;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	float SpawnAnimationElapsed = 0.0f;
};
