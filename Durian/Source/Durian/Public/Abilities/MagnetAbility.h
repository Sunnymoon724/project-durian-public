#pragma once

#include "CoreMinimal.h"
#include "Abilities/Ability.h"

class UPrimitiveComponent;

class FMagnetAbility final : public FAbility
{
public:
	explicit FMagnetAbility(AMyPlayerCharacter* InCharacter);

	virtual void Tick(float DeltaTime) override;
	virtual void HandleInteract() override;
	virtual void HandleCancel() override;
	virtual void HandleAbilityUse() override;
	virtual void HandleDistance(float AxisValue) override;

	void Release();

private:
	void UpdateTargeting();
	void UpdateControl(float DeltaTime);
	void SelectTarget();
	void ExitTargetingMode();
	bool TraceTarget(UPrimitiveComponent*& OutComponent, FVector& OutLocation) const;
	void SetTargetedComponent(UPrimitiveComponent* NewTarget, const FVector& NewTargetLocation);
	void ClearTargetedComponent();

	TWeakObjectPtr<UPrimitiveComponent> TargetedComponent;
	FVector TargetedLocation = FVector::ZeroVector;
	FVector CurrentHoldLocation = FVector::ZeroVector;

	float MagnetDistance = 600.0f;
	float MagnetMinDistance = 250.0f;
	float MagnetMaxDistance = 1200.0f;
	float MagnetFollowSpeed = 5.0f;
};
