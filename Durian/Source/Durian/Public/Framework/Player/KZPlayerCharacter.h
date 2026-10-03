// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Core/PlayerAbilityComponent.h"
#include "CoreMinimal.h"
#include "Enums/PlayerEnums.h"
#include "GameFramework/Character.h"

class UPrimitiveComponent;
class UAbilityEffectComponent;
class UDamageableComponent;
class UTitanClimbingComponent;
class UPointLightComponent;
class UAnimSequence;
class UAnimMontage;
#include "KzPlayerCharacter.generated.h"

UCLASS()
class DURIAN_API AKzPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FAbilityChangedDelegate, EAbilityType);

	// Sets default values for this character's properties
	AKzPlayerCharacter();
	virtual ~AKzPlayerCharacter() override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void StopJumping() override;
	
protected:
private:
	friend class UPlayerAbilityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAbilityEffectComponent> AbilityEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDamageableComponent> Damageable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPlayerAbilityComponent> AbilityComponent;

	/** Wall traversal is supplied by the installed Titan Climbing plugin. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (AllowPrivateAccess = true))
	TObjectPtr<UTitanClimbingComponent> TitanClimbing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPointLightComponent> HoverGlow;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<UAnimSequence> ClimbIdleAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<UAnimSequence> ClimbUpAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<UAnimSequence> ClimbDownAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<UAnimSequence> ClimbLeftAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<UAnimSequence> ClimbRightAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb")
	TObjectPtr<UAnimSequence> ClimbJumpAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Swim")
	TObjectPtr<UAnimSequence> SwimIdleAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Swim")
	TObjectPtr<UAnimSequence> SwimForwardAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Swim")
	TObjectPtr<UAnimSequence> SwimLeftAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Swim")
	TObjectPtr<UAnimSequence> SwimRightAnimation;


	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Hover", meta = (AllowPrivateAccess = true))
	float HoverFallSpeed = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Hover", meta = (AllowPrivateAccess = true))
	float HoverAirControl = 0.7f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EPlayerState CurrentState = EPlayerState::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EAbilityType CurrentAbilityType = EAbilityType::Magnesis;

public:
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void HandleGuard();
	void StopGuard();
	void HandleAttack();
	void PlayBombThrowAnimation();

	void SetAbility(EAbilityType NewAbility) const;
	UPlayerAbilityComponent* GetAbilityComponent() const { return AbilityComponent; }

	FAbilityChangedDelegate OnAbilityChanged;

	UAbilityEffectComponent* GetAbilityEffect() const { return AbilityEffect; }

	EPlayerState GetCurrentState() const { return CurrentState; }
	EAbilityType GetCurrentAbilityType() const { return CurrentAbilityType; }

	/** Space: climb when attached, toggle hover while falling, otherwise jump. */
	void HandleTraversalPressed();
	/** The contextual cancel input releases the wall without starting a sprint. */
	UFUNCTION(BlueprintCallable, Category = "Traversal")
	bool CancelWallClimb();
	/** Right mouse: hold to sprint; release returns to normal walking. */
	void SetSprintRequested(bool bRequested);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void SetTraversalInput(const FVector2D& Input);
	void SetDashDirection(const FVector& Direction);
	/** Starts a distance-matched GASP stop while CharacterMovement is still braking. */
	void HandleMovementReleased();
	bool IsWallClimbing() const;
	bool IsWallClimbJumping() const;
	float GetWallClimbJumpProgress() const;
	bool IsHovering() const { return bHovering; }
	bool IsSurfaceSwimming() const;
	bool IsWaterExitInProgress() const;
	float GetForwardInputValue() const { return ClimbInput.Y; }
	FVector2D GetClimbInput() const { return ClimbInput; }

	UFUNCTION(BlueprintCallable, Category = "State")
	void SetPlayerState(const EPlayerState NewState);

private:
	bool StartHover();
	void StopHover(bool bLanded);
	void PerformSwordHit();
	void RefreshSprintSpeed();
	void UpdateRunStop(float DeltaSeconds);
	void UpdateClimbAnimation();
	void UpdateSurfaceSwimming();
	void CancelRunStop();
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> ActiveClimbAnimation;
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveClimbMontage;
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> ActiveSwimAnimation;
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveSwimMontage;
	bool bWasSwimming = false;
	bool bSavedOrientRotationToMovement = true;
	float SavedStopBrakingDeceleration = 0.0f;
	float SavedStopBrakingFrictionFactor = 0.0f;
	bool bRunStopOwnsBraking = false;
	static constexpr float RunStopDeceleration = 2000.0f;

	float SavedGravityScale = 1.0f;
	float SavedAirControl = 0.0f;
	bool bHovering = false;
	bool bReceivedTraversalInputThisFrame = false;
	FVector LastTraversalDirection = FVector::ZeroVector;
	FVector2D ClimbInput = FVector2D::ZeroVector;
	bool bGuarding = false;
	bool bSprintRequested = false;
	bool bSprinting = false;

	static constexpr float NormalWalkSpeed = 300.0f;
	static constexpr float SprintSpeed = 600.0f;
};
