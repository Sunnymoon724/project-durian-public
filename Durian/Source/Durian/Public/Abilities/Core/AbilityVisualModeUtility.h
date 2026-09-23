#pragma once

#include "Abilities/Core/AbilityModeTypes.h"
#include "Enums/PlayerEnums.h"

class DURIAN_API FAbilityVisualModeUtility final
{
public:
	static EAbilityVisualMode FromAbilityType(EAbilityType Ability);
};
