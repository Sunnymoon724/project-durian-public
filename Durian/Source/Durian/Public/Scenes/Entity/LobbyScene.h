// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Scenes/Core/KZScene.h"
#include "LobbyScene.generated.h"

class UKzHudWidget;

/**
 * 
 */
UCLASS()
class DURIAN_API ULobbyScene : public UKzScene
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWorld* World) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UKzHudWidget> HudWidget;
};
