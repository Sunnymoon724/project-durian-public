#include "Abilities/Core/AbilityVisualModeUtility.h"

EAbilityVisualMode FAbilityVisualModeUtility::FromAbilityType(const EAbilityType AbilityType)
{
	switch (AbilityType)
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
		return EAbilityVisualMode::None;

	default:
		UE_LOG(LogTemp, Log, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(AbilityType));
		return EAbilityVisualMode::None;
	}
}

EAbilityReactionType FAbilityVisualModeUtility::TargetReactionForMode(const EAbilityVisualMode VisualMode)
{
	switch (VisualMode)
	{
	case EAbilityVisualMode::Magnesis:
		return EAbilityReactionType::MagnesisTarget;
	case EAbilityVisualMode::Cryonis:
		return EAbilityReactionType::CryonicTarget;
	case EAbilityVisualMode::Stasis:
		return EAbilityReactionType::StasisTarget;

	case EAbilityVisualMode::None:
		return EAbilityReactionType::Normal;

	default:
		UE_LOG(LogTemp, Log, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(VisualMode));
		return EAbilityReactionType::Normal;
	}
}
