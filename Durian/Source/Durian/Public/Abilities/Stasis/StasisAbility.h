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

	void Tick(float DeltaTime) override;
	void HandleInteract() override;
	void HandleCancel() override;
	void HandleAbilityUse() override;
	void HandleAttackHit(const FHitResult& Hit, const FVector& AttackDirection);
	void AbortForEndPlay();
	bool IsStasisActive() const;
	float GetRemainingTime() const { return RemainingTime; }
	float GetCooldownRemaining() const { return CooldownRemaining; }
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
	float CooldownRemaining = 0.0f;
};
