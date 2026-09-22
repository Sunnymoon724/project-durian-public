#pragma once

#include "Abilities/Core/Ability.h"

class UPrimitiveComponent;

class FStasisAbility final : public FAbility
{
public:
	explicit FStasisAbility(AKzPlayerCharacter* InCharacter)
		: FAbility(InCharacter)
	{
	}

	void Tick(float DeltaTime) override;
	void HandleInteract() override;
	void HandleCancel() override;
	void HandleAbilityUse() override;
	void AccumulateImpulse(const FVector& Impulse);

private:
	void UpdateTargeting();
	void StartStasis();
	void EndStasis(bool bApplyImpulse);
	void ClearTarget();
	bool TraceTarget(UPrimitiveComponent*& OutComponent) const;

	TWeakObjectPtr<UPrimitiveComponent> TargetedComponent;
	FVector SavedLinearVelocity = FVector::ZeroVector;
	FVector SavedAngularVelocity = FVector::ZeroVector;
	FVector AccumulatedImpulse = FVector::ZeroVector;
	float RemainingTime = 0.0f;
	float CooldownRemaining = 0.0f;
};
