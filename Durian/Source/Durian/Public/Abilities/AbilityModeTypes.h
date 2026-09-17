#pragma once

#include "CoreMinimal.h"
#include "AbilityModeTypes.generated.h"

/** 전역으로 전환되는 능력 모드다. */
UENUM(BlueprintType)
enum class EAbilityMode : uint8
{
	None,
	Magnet,
	Ice,
	TimeLock,
	Bomb
};

UENUM(BlueprintType)
enum class EAbilityReactionType : uint8
{
	Normal,
	MagnetTarget,
	TimeLockTarget,
	Water
};
