// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Core/PlayerAbilityComponent.h"
#include "CoreMinimal.h"
#include "Enums/PlayerEnums.h"
#include "GameFramework/Character.h"

class UPrimitiveComponent;
class UAbilityEffectComponent;
class UDamageableComponent;
class UPhysicsHandleComponent;
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
	
protected:
private:
	friend class UPlayerAbilityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magnesis", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UPhysicsHandleComponent> PhysicsHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAbilityEffectComponent> AbilityEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDamageableComponent> Damageable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = true))
	TObjectPtr<UPlayerAbilityComponent> AbilityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EPlayerState CurrentState = EPlayerState::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EAbilityType CurrentAbilityType = EAbilityType::Magnesis;

public:
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void HandleInteract() const;
	void HandleCancel() const;
	void HandleAbilityUse() const;
	void HandleMagnesisDistanceInput(float AxisValue) const;
	void HandleGuard();
	void HandleAttack();
	void HandleIceTargetAtFeet() const;
	void HandleRemoteBombThrow() const;
	void SetAbility(EAbilityType NewAbility);
	FAbilityChangedDelegate OnAbilityChanged;

	UPhysicsHandleComponent* GetPhysicsHandle() const { return PhysicsHandle; }
	UAbilityEffectComponent* GetAbilityEffect() const { return AbilityEffect; }
	UDamageableComponent* GetDamageable() const { return Damageable; }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	float GetRemoteBombSphereCooldown() const { return AbilityComponent ? AbilityComponent->GetRemoteBombCooldown(ERemoteBombShape::Sphere) : 0.0f; }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	float GetRemoteBombCubeCooldown() const { return AbilityComponent ? AbilityComponent->GetRemoteBombCooldown(ERemoteBombShape::Cube) : 0.0f; }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool HasRemoteBombSphere() const { return AbilityComponent && AbilityComponent->HasRemoteBomb(ERemoteBombShape::Sphere); }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool HasRemoteBombCube() const { return AbilityComponent && AbilityComponent->HasRemoteBomb(ERemoteBombShape::Cube); }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsRemoteBombSphereInstalled() const { return AbilityComponent && AbilityComponent->IsRemoteBombInstalled(ERemoteBombShape::Sphere); }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsRemoteBombCubeInstalled() const { return AbilityComponent && AbilityComponent->IsRemoteBombInstalled(ERemoteBombShape::Cube); }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsHoldingRemoteBomb() const { return AbilityComponent && AbilityComponent->IsHoldingRemoteBomb(); }
	EPlayerState GetCurrentState() const { return CurrentState; }
	EAbilityType GetCurrentAbilityType() const { return CurrentAbilityType; }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	bool IsStasisActive() const { return AbilityComponent && AbilityComponent->IsStasisActive(); }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	float GetStasisRemainingTime() const { return AbilityComponent ? AbilityComponent->GetStasisRemainingTime() : 0.0f; }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	float GetStasisCooldownRemaining() const { return AbilityComponent ? AbilityComponent->GetStasisCooldownRemaining() : 0.0f; }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	FVector GetStasisAccumulatedImpulse() const { return AbilityComponent ? AbilityComponent->GetStasisAccumulatedImpulse() : FVector::ZeroVector; }
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetPlayerState(const EPlayerState NewState);

};
