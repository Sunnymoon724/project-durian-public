#pragma once

#include "Abilities/Core/Ability.h"

class UPrimitiveComponent;
class UStasisTargetComponent;
class AActor;

class FStasisAbility final : public FAbility
{
public:
	explicit FStasisAbility(AKzPlayerCharacter* InCharacter)
		: FAbility(InCharacter)
	{
	}

	virtual void Tick(float DeltaTime) override;
	void TickPersistent(float DeltaTime);
	virtual void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) override;
	void HandleAttackHit(const FHitResult& Hit, const FVector& AttackDirection) const;
	void HandleAbilityDeselected();
	void AbortForEndPlay();
	bool IsStasisActive() const;
	float GetRemainingTime() const { return RemainingTime; }
	float GetCooldownRemaining() const;
	FVector GetAccumulatedImpulse() const;

private:
	void UpdateTargeting();
	void StartStasis();
	void EndStasis(bool bApplyImpulse, bool bStartCooldown);
	void ClearTarget();
	bool TraceTarget(UPrimitiveComponent*& OutComponent) const;

	TWeakObjectPtr<UPrimitiveComponent> TargetedComponent;
	TWeakObjectPtr<AActor> AimingMarker;
	TWeakObjectPtr<UStasisTargetComponent> ActiveTarget;
	float RemainingTime = 0.0f;
	double CooldownEndTime = 0.0;
};
