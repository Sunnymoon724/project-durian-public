#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AbilityModeSubsystem.generated.h"

/**
 * Manages global ability mode state and notifies world objects.
 * The player requests mode changes; registered objects handle their own reactions.
 */
UCLASS()
class DURIAN_API UAbilityModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool RegisterAbilityModeListener(UObject* Listener);
	void UnregisterAbilityModeListener(UObject* Listener);
	void SetAbilityModeActive(EAbilityVisualMode VisualMode, bool IsEnabled);
	bool IsAbilityModeActive(EAbilityVisualMode VisualMode) const;

	void SetModeScanDirection(const FVector2D& Direction);
	FVector2D GetModeScanDirection() const { return ModeScanDirection; }

	virtual void Deinitialize() override;

private:
	static void NotifyListener(UObject* Listener, EAbilityVisualMode Mode, bool bEnabled);
	void RemoveInvalidListeners();

	TMap<TWeakObjectPtr<UObject>, TSet<EAbilityVisualMode>> RegisteredListeners;
	TSet<EAbilityVisualMode> ActiveModes;
	FVector2D ModeScanDirection = FVector2D(1.0f, 0.0f);
};
