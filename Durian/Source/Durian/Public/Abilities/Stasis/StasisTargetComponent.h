#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StasisTargetComponent.generated.h"

class UPrimitiveComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UNiagaraComponent;
class AActor;

UCLASS(ClassGroup = (Abilities), meta = (BlueprintSpawnableComponent))
class DURIAN_API UStasisTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool BeginStasis(UPrimitiveComponent* InPrimitive);
	void AccumulateImpulse(const FVector& Impulse, float MaxImpulse);
	void EndStasis(bool bApplyImpulse);
	void UpdateFeedback(float RemainingTime, float TotalTime, const FVector& ViewerLocation, float MaxImpulse);
	UFUNCTION(BlueprintPure, Category = "Stasis")
	bool IsStasisActive() const { return bStasisActive; }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	FVector GetAccumulatedImpulse() const { return AccumulatedImpulse; }
	UPrimitiveComponent* GetFrozenPrimitive() const { return FrozenPrimitive.Get(); }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void CreateFeedback();
	void ClearFeedback();
	void SpawnPulse() const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> MarkerActor;

	UPROPERTY(Transient)
	TObjectPtr<UTextRenderComponent> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> DirectionShaft;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> DirectionTip;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> HoldAmbient;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ExpiryWarning;

	TWeakObjectPtr<UPrimitiveComponent> FrozenPrimitive;
	FVector SavedLinearVelocity = FVector::ZeroVector;
	FVector SavedAngularVelocity = FVector::ZeroVector;
	FVector AccumulatedImpulse = FVector::ZeroVector;
	bool bSavedGravityEnabled = false;
	bool bSavedCanBeDamaged = false;
	bool bStasisActive = false;
};
