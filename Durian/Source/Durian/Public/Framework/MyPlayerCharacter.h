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
class UPhysicsHandleComponent;
#include "MyPlayerCharacter.generated.h"

UCLASS()
class DURIAN_API AMyPlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	
	

public:
	// Sets default values for this character's properties
	AMyPlayerCharacter();
	virtual ~AMyPlayerCharacter() override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magnesis", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UPhysicsHandleComponent> PhysicsHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAbilityEffectComponent> AbilityEffect;

	TUniquePtr<FMagnesisAbility> MagnesisAbility;
	TUniquePtr<FStasisAbility> StasisAbility;
	TUniquePtr<FRemoteBombAbility> RemoteBombAbility;
	TUniquePtr<FCryonisAbility> CryonisAbility;
	FAbility* CurrentAbility = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EPlayerState CurrentState = EPlayerState::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EAbilityType CurrentAbilityType = EAbilityType::Magnesis;

public:
	virtual void Tick(const float DeltaTime) override;

	void HandleInteract() const;
	void HandleCancel() const;
	void HandleAbilityUse() const;
	void HandleMagnesisDistanceInput(float AxisValue) const;
	void HandleGuard() const;
	void HandleIceTargetAtFeet() const;
	void HandleRemoteBombThrow() const;
	void SetAbility(EAbilityType NewAbility);

	UPhysicsHandleComponent* GetPhysicsHandle() const { return PhysicsHandle; }
	UAbilityEffectComponent* GetAbilityEffect() const { return AbilityEffect; }
	EPlayerState GetCurrentState() const { return CurrentState; }
	void SetPlayerState(const EPlayerState NewState);

};
