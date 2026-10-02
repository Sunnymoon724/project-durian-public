#include "World/Trigger/TriggerSource.h"

ATriggerSource::ATriggerSource()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATriggerSource::BroadcastTriggerState(const bool IsTriggered)
{
	if (bIsTriggered == IsTriggered)
	{
		return;
	}

	bIsTriggered = IsTriggered;

	OnTriggerStateChanged.Broadcast(bIsTriggered);
}
