#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TriggerTarget.generated.h"

UINTERFACE(BlueprintType)
class DURIAN_API UTriggerTarget : public UInterface
{
	GENERATED_BODY()
};

/** Implement on actors that react to a trigger source's binary state. */
class DURIAN_API ITriggerTarget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Trigger")
	void OnTriggerStateChanged(bool IsTriggered);
};
