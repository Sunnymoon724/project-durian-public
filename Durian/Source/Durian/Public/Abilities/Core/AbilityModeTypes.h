#pragma once

#include "CoreMinimal.h"
#include "AbilityModeTypes.generated.h"

/** 월드 시각 반응과 타깃 강조에 사용하는 모드다. */
UENUM(BlueprintType)
enum class EAbilityVisualMode : uint8
{
	None,
	Magnesis,
	Cryonis,
	Stasis
};

UENUM(BlueprintType)
enum class EAbilityReactionType : uint8
{
	Normal,
	MagnesisTarget,
	StasisTarget,
	CryonicTarget
};
