#include "Constants/GameConstantsDataAsset.h"

#include "UObject/SoftObjectPath.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/StrongObjectPtr.h"

const UGameConstantsDataAsset* UGameConstantsDataAsset::Get()
{
	static TStrongObjectPtr<UGameConstantsDataAsset> GameConstantsAsset;
	if (!GameConstantsAsset.IsValid())
	{
		UGameConstantsDataAsset* LoadedAsset = TSoftObjectPtr<UGameConstantsDataAsset>(FSoftObjectPath(TEXT("/Game/Data/Global/DA_GameConstants.DA_GameConstants"))).LoadSynchronous();
		GameConstantsAsset.Reset(LoadedAsset);
	}

	return GameConstantsAsset.IsValid() ? GameConstantsAsset.Get() : GetDefault<UGameConstantsDataAsset>();
}
