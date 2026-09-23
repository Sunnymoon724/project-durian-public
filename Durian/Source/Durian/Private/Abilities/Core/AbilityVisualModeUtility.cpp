#include "Abilities/Core/AbilityVisualModeUtility.h"

EAbilityVisualMode FAbilityVisualModeUtility::FromAbilityType(const EAbilityType Ability)
{
	switch (Ability)
	{
	case EAbilityType::Magnesis:
		return EAbilityVisualMode::Magnesis;
	case EAbilityType::Cryonis:
		return EAbilityVisualMode::Cryonis;
	case EAbilityType::Stasis:
		return EAbilityVisualMode::Stasis;
	case EAbilityType::None:
	case EAbilityType::RemoteBombSphere:
	case EAbilityType::RemoteBombCube:
	default:
		return EAbilityVisualMode::None;
	}
}
