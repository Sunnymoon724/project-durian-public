#pragma once

#include "Abilities/Core/Ability.h"
#include "StasisAbility.generated.h"

class UPrimitiveComponent;
class UStasisTargetComponent;
class AActor;

UCLASS(BlueprintType)
class DURIAN_API UStasisAbility final : public UAbility
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	void TickPersistent(float DeltaTime);
	virtual void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) override;
	void HandleAttackHit(const FHitResult& Hit, const FVector& AttackDirection) const;
	void OnDeselected() override;
	void AbortForEndPlay();
	UFUNCTION(BlueprintPure, Category = "Stasis")
	bool IsStasisActive() const;
	UFUNCTION(BlueprintPure, Category = "Stasis")
	float GetRemainingTime() const { return RemainingTime; }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	float GetCooldownRemaining() const;
	UFUNCTION(BlueprintPure, Category = "Stasis")
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
