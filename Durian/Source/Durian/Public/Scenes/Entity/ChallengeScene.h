// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Scenes/Core/KZScene.h"
#include "ChallengeScene.generated.h"

/**
 * 
 */
UCLASS()
class DURIAN_API UChallengeScene : public UKzScene
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWorld* World) override;

};
