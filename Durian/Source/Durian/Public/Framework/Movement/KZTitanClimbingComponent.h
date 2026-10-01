#pragma once

#include "TitanClimbingComponent.h"
#include "KZTitanClimbingComponent.generated.h"

/** Titan traversal with a swept floor check for the complete descent step. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DURIAN_API UKzTitanClimbingComponent : public UTitanClimbingComponent
{
	GENERATED_BODY()

public:
	bool StartClimbingOnTaggedWall();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ConfigureClimbableCollision(class AActor* Actor);
	FDelegateHandle ActorSpawnedHandle;
	bool bClimbTraceConfigured = false;
};
