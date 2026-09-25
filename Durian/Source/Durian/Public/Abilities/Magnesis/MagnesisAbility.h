#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/Ability.h"

class UPrimitiveComponent;

class FMagnesisAbility final : public FAbility
{
public:
	explicit FMagnesisAbility(AKzPlayerCharacter* InCharacter);

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

	float MagnesisDistance = 0.0f;
};
