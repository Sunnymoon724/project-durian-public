// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PlayerAbility.generated.h"

/** Player-selectable abilities used by shrine gameplay. */
UENUM(BlueprintType)
enum class EPlayerAbility : uint8
{
	None,
	Magnet,
	Ice,
	Stasis,
	RemoteBomb
};
