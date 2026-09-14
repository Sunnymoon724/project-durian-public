// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerEnums.generated.h"

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	Normal,
	MagnetTargeting,
	MagnetControl,
	Attack
};

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	None,
	Magnet,
	Ice,
	Stasis,
	Bomb
};
