#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "UObject/Object.h"
#include "Ability.generated.h"

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

UCLASS(Abstract, BlueprintType)
class DURIAN_API UAbility : public UObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(AKzPlayerCharacter* InCharacter) { Character = InCharacter; }

	virtual void Tick(float DeltaTime) {}
	virtual void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) { }
	virtual void OnDeselected();

protected:
	float GetRemainingCooldown(double CooldownEndTime) const;

	void SetAbilityModeActive(EAbilityVisualMode VisualMode, bool IsActive, bool bUpdateScanDirection = false) const;
	void SetAbilityVisionEnabled(EAbilityType Ability, bool bEnabled) const;

	void ClearAbilityVisionEffects() const;
	bool UpdateScanActivation(EAbilityVisualMode Mode, EAbilityType Ability, bool bUpdateScanDirection = false) const;
	bool TraceAbilityTarget(EAbilityReactionType ReactionType, float TraceRange, FHitResult& OutHit, bool bTargetAtFeet = false, const AActor* IgnoredActor = nullptr, bool bRequirePhysics = false) const;

	UPROPERTY(Transient)
	TObjectPtr<AKzPlayerCharacter> Character = nullptr;
};
