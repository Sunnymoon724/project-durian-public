#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "KzSoldierAnimationAssetUtility.generated.h"

/** Repairs the Skeleton references lost when Soldier assets were copied as files. */
UCLASS()
class DURIAN_API UKzSoldierAnimationAssetUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Soldier|Migration")
	static int32 RepairTraversalAnimationSkeletons();
};
