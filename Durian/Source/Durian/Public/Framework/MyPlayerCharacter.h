// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UPrimitiveComponent;
class UAbilityEffectComponent;

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enums/PlayerEnums.h"
#include "MyPlayerCharacter.generated.h"

UCLASS()
class DURIAN_API AMyPlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	
	

public:
	// Sets default values for this character's properties
	AMyPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
	void UpdateMagnetTargeting();
	void SetTargetedMagnetComponent(UPrimitiveComponent* NewTarget, const FVector& NewTargetLocation);
	void ClearTargetedMagnetComponent();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magnet", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UPhysicsHandleComponent> PhysicsHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability VFX", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAbilityEffectComponent> AbilityEffect;
	
	TWeakObjectPtr<UPrimitiveComponent> TargetedMagnetComponent;
	FVector TargetedMagnetLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EPlayerState CurrentState = EPlayerState::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnet", meta = (ClampMin = "100.0", ClampMax = "3000.0", AllowPrivateAccess = true))
	float MagnetDistance = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnet", meta = (AllowPrivateAccess = true))
	float MagnetMinDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magnet", meta = (AllowPrivateAccess = true))
	float MagnetMaxDistance = 1200.0f;

public:
	virtual void Tick(const float DeltaTime) override;

	void HandleInteract();
	void HandleMagnetAction();
	void HandleMagnetSelect();
	void HandleMagnetCancel();
	void HandleMagnetDistance(float AxisValue);
	void ReleaseMagnet();
	void UpdateMagnetControl() const;
	void SetPlayerState(const EPlayerState NewState);
};
