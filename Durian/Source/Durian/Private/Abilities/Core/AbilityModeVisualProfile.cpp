#include "Abilities/Core/AbilityModeVisualProfile.h"

namespace
{
	UAbilityModeVisualProfilesDataAsset* LoadVisualProfilesAsset()
	{
		static TWeakObjectPtr<UAbilityModeVisualProfilesDataAsset> VisualProfilesAsset;
		if (!VisualProfilesAsset.IsValid())
		{
			VisualProfilesAsset = TSoftObjectPtr<UAbilityModeVisualProfilesDataAsset>(FSoftObjectPath(TEXT("/Game/Data/AbilityMode/DA_AbilityModeVisualProfiles.DA_AbilityModeVisualProfiles"))).LoadSynchronous();
		}
		return VisualProfilesAsset.Get();
	}
}

const FAbilityModeVisualCommonProfile& FAbilityModeVisualProfiles::GetCommon()
{
	static const FAbilityModeVisualCommonProfile constexpr EmptyCommon{};
	if (UAbilityModeVisualProfilesDataAsset* VisualProfilesAsset = LoadVisualProfilesAsset())
	{
		return VisualProfilesAsset->Common;
	}

	UE_LOG(LogTemp, Error, TEXT("AbilityModeVisualProfiles: Failed to load common visual profile."));
	
	return EmptyCommon;
}

const FAbilityModeVisualProfile& FAbilityModeVisualProfiles::Get(const EAbilityMode Mode)
{
	static const FAbilityModeVisualProfile constexpr EmptyProfile{};
	UAbilityModeVisualProfilesDataAsset* VisualProfilesAsset = LoadVisualProfilesAsset();
	
	if (!VisualProfilesAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("AbilityModeVisualProfiles: Failed to load Data Asset '/Game/Data/AbilityMode/DA_AbilityModeVisualProfiles'"));
		return EmptyProfile;
	}

	switch (Mode)
	{
	case EAbilityMode::Magnesis: return VisualProfilesAsset->Magnesis;
	case EAbilityMode::Cryonis: return VisualProfilesAsset->Cryonis;
	case EAbilityMode::Stasis: return VisualProfilesAsset->Stasis;
	case EAbilityMode::RemoteBomb: return VisualProfilesAsset->RemoteBomb;
	case EAbilityMode::None:
	default:
		UE_LOG(LogTemp, Error, TEXT("AbilityModeVisualProfiles: Invalid ability mode %s in '/Game/Data/AbilityMode/DA_AbilityModeVisualProfiles'"), *UEnum::GetValueAsString(Mode));
		return EmptyProfile;
	}
}
