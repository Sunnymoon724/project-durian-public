#pragma once

#include "CoreMinimal.h"
#include "AbilityModeTypes.generated.h"

/** 전역으로 전환되는 능력 모드다. */
UENUM(BlueprintType)
enum class EAbilityMode : uint8
{
	None,
	Magnesis,
	Cryonis,
	Stasis,
	RemoteBomb
};

UENUM(BlueprintType)
enum class EAbilityReactionType : uint8
{
	Normal,
	MagnesisTarget,
	StasisTarget,
	CryonicTarget
};
