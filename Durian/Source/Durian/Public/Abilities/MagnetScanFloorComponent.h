#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagnetScanFloorComponent.generated.h"

/**
 * Marks a floor actor for the Magnesis floor scan. The final scan is applied
 * by the world post-process; this marker intentionally does not write a
 * floor-only custom stencil.
 */
UCLASS(ClassGroup=(Ability), meta=(BlueprintSpawnableComponent))
class DURIAN_API UMagnetScanFloorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMagnetScanFloorComponent();

};
