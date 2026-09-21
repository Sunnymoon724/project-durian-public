// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Scenes/Core/KZSceneDefinition.h"
#include "MainConfig.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EStartupMode : uint8
{
	Normal,
	Test,
};

UCLASS(BlueprintType)
class DURIAN_API UMainConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Startup")
	EStartupMode StartupMode = EStartupMode::Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenes")
	TSoftObjectPtr<UKzSceneDefinition> NormalStartupScene;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scenes")
	TSoftObjectPtr<UKzSceneDefinition> TestStartupScene;

	const TSoftObjectPtr<UKzSceneDefinition>& GetStartupScene() const
	{
		return StartupMode == EStartupMode::Test ? TestStartupScene : NormalStartupScene;
	}
};
