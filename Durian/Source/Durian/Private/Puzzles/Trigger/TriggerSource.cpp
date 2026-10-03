#include "Puzzles/Trigger/TriggerSource.h"

ATriggerSource::ATriggerSource()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATriggerSource::SetEnabled(const bool Enabled)
{
	if ((CurrentState != ETriggerSourceState::Off) == Enabled)
	{
		return;
	}

	const ETriggerSourceState NewState = Enabled ? ETriggerSourceState::Ready : ETriggerSourceState::Off;

	SetCurrentState(NewState);
	OnEnabledChanged(Enabled);
}

void ATriggerSource::Initialize()
{
	bInitialized = true;

	SetCurrentState(StartState);
}

void ATriggerSource::SetCurrentState(const ETriggerSourceState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const bool bWasTriggered = GetIsTriggered();
	CurrentState = NewState;
	const bool bNewStateIsTriggered = GetIsTriggered();
	if (bWasTriggered != bNewStateIsTriggered)
	{
		OnTriggerStateChanged.Broadcast(bNewStateIsTriggered);
	}
}
