#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KzSceneTransitionComponent.generated.h"

class UKzSceneDefinition;

UCLASS(ClassGroup=(Scene), meta=(BlueprintSpawnableComponent))
class DURIAN_API UKzSceneTransitionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKzSceneTransitionComponent();

	UFUNCTION(BlueprintCallable, Category="Scene Transition")
	bool Transition();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scene Transition")
	TObjectPtr<UKzSceneDefinition> NextSceneDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Scene Transition")
	FName CompletedChallengeId = NAME_None;
};
