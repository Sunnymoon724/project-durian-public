#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KzSceneDefinition.generated.h"

class UKzScene;
class UUserWidget;
class UInputMappingContext;

UCLASS(BlueprintType)
class DURIAN_API UKzSceneDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scene")
	TSoftObjectPtr<UWorld> TargetMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scene")
	TSubclassOf<UKzScene> SceneClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scene|Resources")
	TArray<TSoftObjectPtr<UObject>> PreloadAssets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scene|UI")
	TArray<TSoftClassPtr<UUserWidget>> DefaultWidgets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scene|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
};
