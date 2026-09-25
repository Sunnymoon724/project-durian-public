// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/Cryonis/CryonisAbility.h"
#include "Abilities/Magnesis/MagnesisAbility.h"
#include "Abilities/RemoteBomb/RemoteBombAbility.h"
#include "Abilities/Stasis/StasisAbility.h"
#include "CoreMinimal.h"
#include "Enums/PlayerEnums.h"
#include "GameFramework/Character.h"

class UPrimitiveComponent;
class UAbilityEffectComponent;
class UDamageableComponent;
class ARemoteBomb;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magnesis", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UPhysicsHandleComponent> PhysicsHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAbilityEffectComponent> AbilityEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = true))
	TObjectPtr<UDamageableComponent> Damageable;

	UPROPERTY(EditDefaultsOnly, Category = "Remote Bomb")
	TSubclassOf<ARemoteBomb> RemoteBombClass;

	TUniquePtr<FMagnesisAbility> MagnesisAbility;
	TUniquePtr<FStasisAbility> StasisAbility;
	TUniquePtr<FRemoteBombAbility> RemoteBombAbility;
	TUniquePtr<FCryonisAbility> CryonisAbility;
	FAbility* CurrentAbility = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EPlayerState CurrentState = EPlayerState::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EAbilityType CurrentAbilityType = EAbilityType::Magnesis;
	void InterruptMagnesis();
	void InterruptCryonis();
	void InterruptStasisTargeting();

public:
	virtual void Tick(const float DeltaTime) override;
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
	TSubclassOf<ARemoteBomb> GetRemoteBombClass() const { return RemoteBombClass; }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	float GetRemoteBombSphereCooldown() const { return RemoteBombAbility ? RemoteBombAbility->GetCooldownRemaining(ERemoteBombShape::Sphere) : 0.0f; }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	float GetRemoteBombCubeCooldown() const { return RemoteBombAbility ? RemoteBombAbility->GetCooldownRemaining(ERemoteBombShape::Cube) : 0.0f; }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool HasRemoteBombSphere() const { return RemoteBombAbility && RemoteBombAbility->HasBomb(ERemoteBombShape::Sphere); }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool HasRemoteBombCube() const { return RemoteBombAbility && RemoteBombAbility->HasBomb(ERemoteBombShape::Cube); }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsRemoteBombSphereInstalled() const { return RemoteBombAbility && RemoteBombAbility->IsBombInstalled(ERemoteBombShape::Sphere); }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsRemoteBombCubeInstalled() const { return RemoteBombAbility && RemoteBombAbility->IsBombInstalled(ERemoteBombShape::Cube); }
	UFUNCTION(BlueprintPure, Category = "Remote Bomb")
	bool IsHoldingRemoteBomb() const { return RemoteBombAbility && RemoteBombAbility->IsHoldingBomb(); }
	EPlayerState GetCurrentState() const { return CurrentState; }
	EAbilityType GetCurrentAbilityType() const { return CurrentAbilityType; }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	bool IsStasisActive() const { return StasisAbility && StasisAbility->IsStasisActive(); }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	float GetStasisRemainingTime() const { return StasisAbility ? StasisAbility->GetRemainingTime() : 0.0f; }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	float GetStasisCooldownRemaining() const { return StasisAbility ? StasisAbility->GetCooldownRemaining() : 0.0f; }
	UFUNCTION(BlueprintPure, Category = "Stasis")
	FVector GetStasisAccumulatedImpulse() const { return StasisAbility ? StasisAbility->GetAccumulatedImpulse() : FVector::ZeroVector; }
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetPlayerState(const EPlayerState NewState);

};
