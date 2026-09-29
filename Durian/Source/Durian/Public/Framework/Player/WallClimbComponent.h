#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallClimbComponent.generated.h"

class UAnimSequenceBase;

UCLASS(ClassGroup=(Soldier), meta=(BlueprintSpawnableComponent))
class DURIAN_API UWallClimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWallClimbComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Attempts a wall climb and returns true only when a valid wall is directly ahead. */
	bool TryWallClimb();
	/** Starts a high-wall climb after a jump reaches a wall while forward input is held. */
	bool TryStartTallWallClimb();
	/** Continues a Titan climb at a valid ledge with the original Soldier finish animation. */
	bool TryFinishTitanClimb();
	/** Releases a hanging Soldier without treating the second Space press as another jump. */
	bool CancelTallWallClimb();

private:
	bool StartClimb(const FVector& Target, const FVector& Apex, UAnimSequenceBase* Sequence, float Duration, bool bTallWall);
	void FinishWallClimb(bool bReachedTop = true);

	UPROPERTY(EditAnywhere, Category="Wall Climb")
	float MinimumWallHeight = 120.f;

	UPROPERTY(EditAnywhere, Category="Wall Climb")
	float MaximumWallHeight = 320.f;

	UPROPERTY(EditAnywhere, Category="Wall Climb")
	float ClimbDuration = 1.25f;

	UPROPERTY(EditAnywhere, Category="Wall Climb")
	float TallWallClimbDuration = 2.5f;

	UPROPERTY(EditAnywhere, Category="Wall Climb")
	float LandingForwardOffset = 45.f;

	UPROPERTY(EditAnywhere, Category="Wall Climb")
	TObjectPtr<class UAnimSequenceBase> VaultSequence;

	UPROPERTY(EditAnywhere, Category="Wall Climb")
	TObjectPtr<class UAnimSequenceBase> TallWallClimbSequence;

	bool bClimbing = false;
	bool bTallWallClimb = false;
	float ElapsedClimbTime = 0.f;
	float ActiveClimbDuration = 0.f;
	float ClimbRetryCooldown = 0.f;
	FVector ClimbStart = FVector::ZeroVector;
	FVector ClimbApex = FVector::ZeroVector;
	FVector ClimbTarget = FVector::ZeroVector;
	TSubclassOf<class UAnimInstance> SavedAnimInstanceClass;
};


