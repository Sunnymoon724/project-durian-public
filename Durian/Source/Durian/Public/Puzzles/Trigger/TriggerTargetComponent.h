#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Puzzles/Trigger/TriggerSource.h"
#include "TriggerTargetComponent.generated.h"

UCLASS(ClassGroup = (Trigger), meta = (BlueprintSpawnableComponent))
class DURIAN_API UTriggerTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTriggerTargetComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleTriggerStateChanged(bool IsTriggered) const;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Trigger", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ATriggerSource> Source;
};
