#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/AbilityModeTypes.h"

class AKzPlayerCharacter;
class AActor;
class UPrimitiveComponent;
struct FHitResult;

enum class EAbilityType : uint8;

enum class EAbilityInput : uint8
{
	Interact,
	Cancel,
	Use,
	CryonisTargetAtFeet,
	RemoteBombThrow,
	MagnesisDistance,
	Interrupt
};

class FAbility
{
public:
	explicit FAbility(AKzPlayerCharacter* InCharacter) : Character(InCharacter) { }
	virtual ~FAbility() = default;

	virtual void Tick(float DeltaTime) {}
	virtual void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) {}

protected:
	float GetRemainingCooldown(double CooldownEndTime) const;
	void SetAbilityModeActive(EAbilityVisualMode Mode, bool bActive, bool bUpdateScanDirection = false) const;
	void SetAbilityVisionEnabled(EAbilityType Ability, bool bEnabled) const;
	void ClearAbilityVisionEffects() const;
	bool UpdateScanActivation(EAbilityVisualMode Mode, EAbilityType Ability, bool bUpdateScanDirection = false) const;
	bool TraceAbilityTarget(EAbilityReactionType ReactionType, float TraceRange, FHitResult& OutHit, bool bTargetAtFeet = false, const AActor* IgnoredActor = nullptr, bool bRequirePhysics = false) const;

	AKzPlayerCharacter* Character = nullptr;
};
