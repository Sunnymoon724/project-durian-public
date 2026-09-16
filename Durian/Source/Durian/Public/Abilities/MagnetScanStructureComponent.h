#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagnetScanStructureComponent.generated.h"

/**
 * Marks a fixed architectural actor for the Magnesis structure outline.
 * Attach this to walls, columns, rails and fixed frames. Floors use
 * UMagnetScanFloorComponent instead.
 */
UCLASS(ClassGroup=(Ability), meta=(BlueprintSpawnableComponent))
class DURIAN_API UMagnetScanStructureComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	static constexpr int32 StructureStencilValue = 4;
};
