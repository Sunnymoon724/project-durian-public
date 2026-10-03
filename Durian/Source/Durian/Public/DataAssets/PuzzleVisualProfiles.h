#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PuzzleVisualProfiles.generated.h"

USTRUCT(BlueprintType)
struct FTriggerSourcePlateVisualProfile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Off")
	FLinearColor OffColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ready")
	FLinearColor ReadyColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Active")
	FLinearColor ActiveColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Emissive", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float EmissivePower = 20.0f;
};

UCLASS(BlueprintType)
class DURIAN_API UPuzzleVisualProfilesDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger Source Plate")
	FTriggerSourcePlateVisualProfile TriggerSourcePlate;
};

class DURIAN_API FPuzzleVisualProfiles final
{
public:
	static const FTriggerSourcePlateVisualProfile& GetTriggerSourcePlate();
};
