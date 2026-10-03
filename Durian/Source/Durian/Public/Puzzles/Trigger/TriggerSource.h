#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriggerSource.generated.h"

UENUM(BlueprintType)
enum class ETriggerSourceState : uint8
{
	Off,
	Ready,
	Active
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTriggerSignalChanged, bool, IsTriggered);

/** Base actor that broadcasts a binary trigger signal to subscribed targets. */
UCLASS(Abstract, Blueprintable)
class DURIAN_API ATriggerSource : public AActor
{
	GENERATED_BODY()

public:
	ATriggerSource();

	void SetEnabled(bool Enabled);

	ETriggerSourceState GetCurrentState() const { return CurrentState; }
	bool GetIsTriggered() const { return CurrentState == ETriggerSourceState::Active; }
	FTriggerSignalChanged OnTriggerStateChanged;

protected:
	virtual void Initialize();

	virtual void SetCurrentState(const ETriggerSourceState NewState);

	virtual void OnEnabledChanged(bool bEnabled) { };

	bool IsEnable() const { return bInitialized && CurrentState != ETriggerSourceState::Off; }
	bool IsInitialized() const { return bInitialized; }

private:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Trigger", meta = (AllowPrivateAccess = "true"))
	ETriggerSourceState StartState = ETriggerSourceState::Ready;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Trigger", meta = (AllowPrivateAccess = "true"))
	ETriggerSourceState CurrentState = StartState;

	bool bInitialized = false;
};
