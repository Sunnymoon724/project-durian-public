#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MagnetScanFloorComponent.generated.h"

/**
 * Marks a floor actor for the Magnesis floor scan. The stencil value is kept
 * internal so level authors only need to attach this component.
 */
UCLASS(ClassGroup=(Ability), meta=(BlueprintSpawnableComponent))
class DURIAN_API UMagnetScanFloorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMagnetScanFloorComponent();

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

private:
	void ApplyFloorStencil();

	static constexpr int32 FloorStencilValue = 5;
};
