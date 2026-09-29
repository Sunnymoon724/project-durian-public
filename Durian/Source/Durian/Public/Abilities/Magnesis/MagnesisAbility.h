#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/Ability.h"

class UPrimitiveComponent;
class UPhysicsHandleComponent;
class UNiagaraComponent;

class FMagnesisAbility final : public FAbility
{
public:
	explicit FMagnesisAbility(AKzPlayerCharacter* InCharacter);

	virtual void Tick(float DeltaTime) override;
	virtual void HandleInput(EAbilityInput Input, float AxisValue = 0.0f) override;

	void Release();

private:
	void EnterTargetingMode();
	void UpdateTargeting();
	void UpdateControl(float DeltaTime);
	void SelectTarget();
	void ExitTargetingMode();
	bool TraceTarget(UPrimitiveComponent*& OutComponent, FVector& OutLocation) const;
	void SetTargetedComponent(UPrimitiveComponent* NewTarget, const FVector& NewTargetLocation);
	void ClearTargetedComponent();
	void UpdateHoldBeam();
	void StopHoldBeam();

	TWeakObjectPtr<UPrimitiveComponent> TargetedComponent;
	FVector TargetedLocation = FVector::ZeroVector;
	FVector CurrentHoldLocation = FVector::ZeroVector;

	float MagnesisDistance = 0.0f;
	UPhysicsHandleComponent* PhysicsHandle = nullptr;
	TWeakObjectPtr<UNiagaraComponent> HoldBeam;
};
