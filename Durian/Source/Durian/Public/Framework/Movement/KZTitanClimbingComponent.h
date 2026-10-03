#pragma once

#include "TitanClimbingComponent.h"
#include "KZTitanClimbingComponent.generated.h"

/** Titan traversal with a swept floor check for the complete descent step. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DURIAN_API UKzTitanClimbingComponent : public UTitanClimbingComponent
{
	GENERATED_BODY()

public:
	UKzTitanClimbingComponent();
	bool StartClimbingOnTaggedWall();
	bool TryStartSwimmingExit();
	bool IsSwimmingExitActive() const { return bSwimmingExit; }
	void CancelSwimmingExit();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ConfigureClimbableCollision(class AActor* Actor);
	void TickSwimmingExit();
	void FinishSwimmingExit(bool bLanded);
	UPROPERTY(EditDefaultsOnly, Category = "Swimming")
	TObjectPtr<class UAnimSequence> SwimmingExitAnimation;
	UPROPERTY(Transient)
	TObjectPtr<class UAnimMontage> SwimmingExitMontage;
	TWeakObjectPtr<AActor> SwimmingExitBank;
	FVector SwimmingExitStart = FVector::ZeroVector;
	FVector SwimmingExitGrip = FVector::ZeroVector;
	FVector SwimmingExitRootStart = FVector::ZeroVector;
	FVector SwimmingExitCorrection = FVector::ZeroVector;
	FVector SwimmingExitSafeLocation = FVector::ZeroVector;
	FQuat SwimmingExitRotation = FQuat::Identity;
	bool bSavedSwimmingOrientation = false;
	bool bBankWasIgnored = false;
	FDelegateHandle ActorSpawnedHandle;
	bool bClimbTraceConfigured = false;
	bool bSwimmingExit = false;
	float SwimmingExitStartTime = 0.0f;
	float NextSwimmingExitTime = 0.0f;
};
