#include "Abilities/AbilityModeSubsystem.h"

#include "Abilities/AbilityModeListener.h"

bool UAbilityModeSubsystem::RegisterAbilityModeListener(UObject* Listener)
{
	if (!IsValid(Listener) || !Listener->GetClass()->ImplementsInterface(UAbilityModeListener::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("AbilityModeSubsystem: '%s' does not implement AbilityModeListener."), *GetNameSafe(Listener));
		return false;
	}

	TSet<EAbilityMode> SupportedModes;
	for (const EAbilityMode Mode : IAbilityModeListener::Execute_GetSupportedAbilityModes(Listener))
	{
		if (Mode != EAbilityMode::None)
		{
			SupportedModes.Add(Mode);
		}
	}

	RegisteredListeners.Add(Listener, MoveTemp(SupportedModes));

	for (const EAbilityMode Mode : ActiveModes)
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

void UAbilityModeSubsystem::SetAbilityModeActive(const EAbilityMode Mode, const bool bEnabled)
{
	if (Mode == EAbilityMode::None)
	{
		return;
	}

	const bool bWasActive = ActiveModes.Contains(Mode);
	if (bWasActive == bEnabled)
	{
		return;
	}

	if (bEnabled)
	{
		ActiveModes.Add(Mode);
	}
	else
	{
		ActiveModes.Remove(Mode);
	}

	RemoveInvalidListeners();

	for (const TPair<TWeakObjectPtr<UObject>, TSet<EAbilityMode>>& Entry : RegisteredListeners)
	{
		if (UObject* Listener = Entry.Key.Get(); Listener && Entry.Value.Contains(Mode))
		{
			NotifyListener(Listener, Mode, bEnabled);
		}
	}
}

bool UAbilityModeSubsystem::IsAbilityModeActive(const EAbilityMode Mode) const
{
	return ActiveModes.Contains(Mode);
}

void UAbilityModeSubsystem::Deinitialize()
{
	RegisteredListeners.Empty();
	ActiveModes.Empty();
	Super::Deinitialize();
}

void UAbilityModeSubsystem::NotifyListener(UObject* Listener, const EAbilityMode Mode, const bool bEnabled) const
{
	IAbilityModeListener::Execute_OnAbilityModeChanged(Listener, Mode, bEnabled);
}

void UAbilityModeSubsystem::RemoveInvalidListeners()
{
	for (auto It = RegisteredListeners.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}
}
