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
class UWallClimbComponent;
class UAnimMontage;
class UAnimSequence;
class UPointLightComponent;
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
	TObjectPtr<UWallClimbComponent> WallClimb;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traversal", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPointLightComponent> HoverGlow;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Hover", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimSequence> HoverAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Dash", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimSequence> DashAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimSequence> ClimbIdleAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimSequence> ClimbUpAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimSequence> ClimbDownAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimSequence> ClimbLeftAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Traversal|Climb", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimSequence> ClimbRightAnimation;

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

	void SetAbility(EAbilityType NewAbility) const;
	UPlayerAbilityComponent* GetAbilityComponent() const { return AbilityComponent; }

	FAbilityChangedDelegate OnAbilityChanged;

	UAbilityEffectComponent* GetAbilityEffect() const { return AbilityEffect; }

	EPlayerState GetCurrentState() const { return CurrentState; }
	EAbilityType GetCurrentAbilityType() const { return CurrentAbilityType; }

	/** Space: climb when attached, toggle hover while falling, otherwise jump. */
	void HandleTraversalPressed();
	/** Right mouse: hold to sprint; release returns to normal walking. */
	void SetSprintRequested(bool bRequested);

	UFUNCTION(BlueprintCallable, Category = "Traversal")
	void SetTraversalInput(const FVector2D& Input);
	void SetDashDirection(const FVector& Direction);
	bool IsWallClimbing() const;
	bool IsHovering() const { return bHovering; }
	float GetForwardInputValue() const { return ClimbInput.Y; }

	UFUNCTION(BlueprintCallable, Category = "State")
	void SetPlayerState(const EPlayerState NewState);

private:
	bool StartHover();
	void StopHover(bool bLanded);
	void UpdateClimbAnimation();
	void PlayClimbAnimation(UAnimSequence* Animation);
	void PerformSwordHit();
	void RefreshSprintSpeed();

	float SavedGravityScale = 1.0f;
	float SavedAirControl = 0.0f;
	bool bHovering = false;
	bool bWasWallClimbing = false;
	bool bReceivedTraversalInputThisFrame = false;
	FVector LastTraversalDirection = FVector::ZeroVector;
	FVector2D ClimbInput = FVector2D::ZeroVector;
	TObjectPtr<UAnimSequence> ActiveClimbAnimation;
	TObjectPtr<UAnimMontage> ActiveClimbMontage;
	TObjectPtr<UAnimSequence> SwordAttack0Animation;
	TObjectPtr<UAnimSequence> SwordAttack1Animation;
	TObjectPtr<UAnimSequence> SwordBlockAnimation;
	TObjectPtr<UAnimMontage> ActiveCombatMontage;
	bool bGuarding = false;
	bool bUseSecondSwordAttack = false;
	bool bSprintRequested = false;
	bool bSprinting = false;

	static constexpr float NormalWalkSpeed = 300.0f;
	static constexpr float SprintSpeed = 600.0f;
};
