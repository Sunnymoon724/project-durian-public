#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KzScene.generated.h"

UCLASS(Abstract, Blueprintable)
class DURIAN_API UKzScene : public UObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWorld* World);
	virtual void Deinitialize();
};
