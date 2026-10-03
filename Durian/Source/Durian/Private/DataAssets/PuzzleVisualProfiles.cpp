#include "DataAssets/PuzzleVisualProfiles.h"

namespace
{
	UPuzzleVisualProfilesDataAsset* LoadPuzzleVisualProfilesAsset()
	{
		static TWeakObjectPtr<UPuzzleVisualProfilesDataAsset> PuzzleVisualProfilesAsset;

		if (!PuzzleVisualProfilesAsset.IsValid())
		{
			PuzzleVisualProfilesAsset = TSoftObjectPtr<UPuzzleVisualProfilesDataAsset>(FSoftObjectPath(TEXT("/Game/Data/Puzzles/DA_PuzzleVisualProfiles.DA_PuzzleVisualProfiles"))).LoadSynchronous();
		}

		return PuzzleVisualProfilesAsset.Get();
	}
}

const FTriggerSourcePlateVisualProfile& FPuzzleVisualProfiles::GetTriggerSourcePlate()
{
	static const FTriggerSourcePlateVisualProfile EmptyProfile{};

	if (const UPuzzleVisualProfilesDataAsset* VisualProfilesAsset = LoadPuzzleVisualProfilesAsset())
	{
		return VisualProfilesAsset->TriggerSourcePlate;
	}

	UE_LOG(LogTemp, Error, TEXT("PuzzleVisualProfiles: Failed to load Data Asset '/Game/Data/Puzzles/DA_PuzzleVisualProfiles'."));

	return EmptyProfile;
}
