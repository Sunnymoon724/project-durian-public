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

	/** Inserts an upper-body-only slot without replacing the existing locomotion graph or parent. */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Animation")
	static bool InstallMagnesisUpperBodyLayer();

	UFUNCTION(BlueprintPure, Category = "Soldier|Animation")
	static bool IsMagnesisHolding(const class UAnimInstance* Animation);

	UFUNCTION(BlueprintCallable, Category = "Soldier|Animation")
	static bool InstallMagnesisStrafeBlend();

	/** Assemble the approved scan/bomb clips on the existing upper-body slot. */
	UFUNCTION(BlueprintCallable, Category = "Soldier|Animation")
	static bool InstallScanAndBombMontages();

};
