#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KzSceneDefinition.generated.h"

class UKzScene;

UCLASS(BlueprintType)
class DURIAN_API UKzSceneDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scene")
	TSoftObjectPtr<UWorld> TargetMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scene")
	TSubclassOf<UKzScene> SceneClass;
};
