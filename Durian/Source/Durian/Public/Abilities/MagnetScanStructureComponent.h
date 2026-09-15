#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagnetScanStructureComponent.generated.h"

/**
 * Marks an architectural actor as part of Magnesis vision.
 * Attach this only to static structure: floors, walls, columns, rails and frames.
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
