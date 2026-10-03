#include "Puzzles/Trigger/TriggerTargetComponent.h"

#include "DrawDebugHelpers.h"
#include "Puzzles/Trigger/TriggerTarget.h"

UTriggerTargetComponent::UTriggerTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTriggerTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(Source) || !GetOwner() || !GetOwner()->GetClass()->ImplementsInterface(UTriggerTarget::StaticClass()))
	{
		return;
	}

	Source->OnTriggerStateChanged.AddDynamic(this, &UTriggerTargetComponent::HandleTriggerStateChanged);

	// A late subscriber only needs synchronization when the source is currently active.
	// The inactive state is each target Blueprint's default and is intentionally silent.
	if (Source->GetIsTriggered())
	{
		HandleTriggerStateChanged(true);
	}
}

void UTriggerTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(Source))
	{
		Source->OnTriggerStateChanged.RemoveDynamic(this, &UTriggerTargetComponent::HandleTriggerStateChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UTriggerTargetComponent::HandleTriggerStateChanged(const bool IsTriggered) const
{
	AActor* Owner = GetOwner();

	if (!IsValid(Owner) || !Owner->GetClass()->ImplementsInterface(UTriggerTarget::StaticClass()))
	{
		return;
	}

	ITriggerTarget::Execute_OnTriggerStateChanged(Owner, IsTriggered);

	if (IsValid(Source))
	{
		const FColor LineColor = IsTriggered ? FColor::Green : FColor::Red;

		DrawDebugLine(GetWorld(), Source->GetActorLocation(), Owner->GetActorLocation(), LineColor, false, 1.0f, 0, 2.0f);
	}
}
