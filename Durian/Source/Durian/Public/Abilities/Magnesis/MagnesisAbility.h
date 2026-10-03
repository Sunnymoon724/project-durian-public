#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/Ability.h"
#include "Engine/EngineTypes.h"
#include "MagnesisAbility.generated.h"

class UPrimitiveComponent;
class UPhysicsHandleComponent;
class UNiagaraComponent;

UCLASS(BlueprintType)
class DURIAN_API UMagnesisAbility final : public UAbility
{
	GENERATED_BODY()

public:
	virtual void Initialize(AKzPlayerCharacter* InCharacter) override;

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
	void RestoreHeldPawnCollision();

	TWeakObjectPtr<UPrimitiveComponent> TargetedComponent;
	TWeakObjectPtr<UPrimitiveComponent> HeldComponent;
	TEnumAsByte<ECollisionResponse> HeldPawnCollisionResponse = ECR_Block;
	FVector TargetedLocation = FVector::ZeroVector;
	FVector CurrentHoldLocation = FVector::ZeroVector;

	float MagnesisDistance = 0.0f;
	UPROPERTY(Transient)
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle = nullptr;
	TWeakObjectPtr<UNiagaraComponent> HoldBeam;
};
