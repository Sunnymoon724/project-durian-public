#pragma once

#include "Animation/AnimInstance.h"
#include "KZPlayerAnimInstance.generated.h"

/**
 * Runtime state consumed by the player animation blueprint.
 * Gameplay state remains owned by AKzPlayerCharacter.
 */
UCLASS(BlueprintType, Blueprintable)
class DURIAN_API UKZPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UKZPlayerAnimInstance();

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void PlayHoverAnimation();
	void StopHoverAnimation();
	void PlayBombThrowAnimation();
	bool StartRunStop(float CurrentSpeed, float Deceleration);
	bool UpdateRunStop(float CurrentSpeed, float Deceleration, float DeltaSeconds);
	void StopRunStop(float BlendOutTime = 0.12f);

	UPROPERTY(BlueprintReadOnly, Category = "Player|Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Movement")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Traversal")
	bool bIsWallClimbing = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Traversal")
	bool bIsHovering = false;

	UPROPERTY(BlueprintReadOnly, Category = "Player|Traversal")
	float ClimbForwardInput = 0.0f;

private:
	void UpdateClimbAnimation(const class AKzPlayerCharacter& Player);
	void PlayClimbAnimation(class UAnimSequence* Animation, bool bLoop = true);

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Hover")
	TObjectPtr<class UAnimSequence> HoverAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<class UAnimSequence> ClimbIdleAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<class UAnimSequence> ClimbUpAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<class UAnimSequence> ClimbDownAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<class UAnimSequence> ClimbLeftAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<class UAnimSequence> ClimbRightAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<class UAnimSequence> ClimbJumpAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TObjectPtr<class UAnimSequence> BombThrowAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Run Stop")
	TObjectPtr<class UAnimSequence> RunStopLeftAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Run Stop")
	TObjectPtr<class UAnimSequence> RunStopRightAnimation;

	UPROPERTY(Transient)
	TObjectPtr<class UAnimSequence> ActiveClimbAnimation;

	UPROPERTY(Transient)
	TObjectPtr<class UAnimMontage> ActiveClimbMontage;

	UPROPERTY(Transient)
	TObjectPtr<class UAnimMontage> ActiveHoverMontage;

	UPROPERTY(Transient)
	TObjectPtr<class UAnimMontage> ActiveCombatMontage;

	UPROPERTY(Transient)
	TObjectPtr<class UAnimSequence> ActiveRunStopAnimation;

	UPROPERTY(Transient)
	TObjectPtr<class UAnimMontage> ActiveRunStopMontage;

	bool bWasWallClimbing = false;
	float RunStopTime = 0.0f;

	float FindRunStopTime(const class UAnimSequence* Animation, float RemainingDistance) const;
	float GetRunStopPoseCost(const class UAnimSequence* Animation, float Time) const;
};
