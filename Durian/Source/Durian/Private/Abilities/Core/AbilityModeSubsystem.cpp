#include "Abilities/Core/AbilityModeSubsystem.h"

#include "Abilities/Core/AbilityModeListener.h"

bool UAbilityModeSubsystem::RegisterAbilityModeListener(UObject* Listener)
{
	if (!IsValid(Listener) || !Listener->GetClass()->ImplementsInterface(UAbilityModeListener::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityModeSubsystem: '%s' does not implement AbilityModeListener."), *GetNameSafe(Listener));

		return false;
	}

	TSet<EAbilityVisualMode> SupportedModeSet;

	for (const EAbilityVisualMode Mode : IAbilityModeListener::Execute_GetSupportedAbilityModes(Listener))
	{
		if (Mode != EAbilityVisualMode::None)
		{
			SupportedModeSet.Add(Mode);
		}
	}

	RegisteredListeners.Add(Listener, MoveTemp(SupportedModeSet));

	for (const EAbilityVisualMode Mode : ActiveModes)
	{
		if (RegisteredListeners.FindChecked(Listener).Contains(Mode))
		{
			NotifyListener(Listener, Mode, true);
		}
	}

	return true;
}

void UAbilityModeSubsystem::UnregisterAbilityModeListener(UObject* Listener)
{
	if (Listener)
	{
		RegisteredListeners.Remove(Listener);
	}
}

void UAbilityModeSubsystem::SetAbilityModeActive(const EAbilityVisualMode VisualMode, const bool IsEnabled)
{
	if (VisualMode == EAbilityVisualMode::None)
	{
		return;
	}

	const bool bWasActive = ActiveModes.Contains(VisualMode);

	if (bWasActive == IsEnabled)
	{
		return;
	}

	if (IsEnabled)
	{
		ActiveModes.Add(VisualMode);
	}
	else
	{
		ActiveModes.Remove(VisualMode);
	}

	RemoveInvalidListeners();

	for (const TPair<TWeakObjectPtr<UObject>, TSet<EAbilityVisualMode>>& Entry : RegisteredListeners)
	{
		if (UObject* Listener = Entry.Key.Get(); Listener && Entry.Value.Contains(VisualMode))
		{
			NotifyListener(Listener, VisualMode, IsEnabled);
		}
	}
}

bool UAbilityModeSubsystem::IsAbilityModeActive(const EAbilityVisualMode VisualMode) const
{
	return ActiveModes.Contains(VisualMode);
}

void UAbilityModeSubsystem::SetModeScanDirection(const FVector2D& Direction)
{
	ModeScanDirection = Direction.GetSafeNormal();

	if (ModeScanDirection.IsNearlyZero())
	{
		ModeScanDirection = FVector2D(1.0f, 0.0f);
	}
}

void UAbilityModeSubsystem::Deinitialize()
{
	RegisteredListeners.Empty();
	ActiveModes.Empty();

	ModeScanDirection = FVector2D(1.0f, 0.0f);

	Super::Deinitialize();
}

void UAbilityModeSubsystem::NotifyListener(UObject* Listener, const EAbilityVisualMode Mode, const bool bEnabled)
{
	IAbilityModeListener::Execute_OnAbilityModeChanged(Listener, Mode, bEnabled);
}

void UAbilityModeSubsystem::RemoveInvalidListeners()
{
	for (auto Iterator = RegisteredListeners.CreateIterator(); Iterator; ++Iterator)
	{
		if (!Iterator.Key().IsValid())
		{
			Iterator.RemoveCurrent();
		}
	}
}
