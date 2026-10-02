#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriggerSource.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTriggerSignalChanged, bool, IsTriggered);

/** Base actor that broadcasts a binary trigger signal to subscribed targets. */
UCLASS(Abstract, Blueprintable)
class DURIAN_API ATriggerSource : public AActor
{
	GENERATED_BODY()

public:
	ATriggerSource();

	UFUNCTION(BlueprintPure, Category = "Trigger")
	bool GetIsTriggered() const { return bIsTriggered; }

	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void BroadcastTriggerState(bool IsTriggered);

	UPROPERTY(BlueprintAssignable, Category = "Trigger")
	FTriggerSignalChanged OnTriggerStateChanged;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Trigger")
	bool bIsTriggered = false;
};
