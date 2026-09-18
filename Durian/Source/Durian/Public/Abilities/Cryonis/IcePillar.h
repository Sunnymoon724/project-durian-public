#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IcePillar.generated.h"

class UAbilityReactionComponent;
class UStaticMeshComponent;
class UNiagaraSystem;

UCLASS()
class DURIAN_API AIcePillar : public AActor
{
	GENERATED_BODY()

public:
	AIcePillar();

	static constexpr float HalfHeight = 100.0f;

	void PlayDestroyEffect();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ice")
	TObjectPtr<UStaticMeshComponent> IceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ice")
	TObjectPtr<UAbilityReactionComponent> ReactionComponent;
};
