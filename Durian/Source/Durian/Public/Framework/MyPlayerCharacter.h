// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UPrimitiveComponent;

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

protected:
	virtual void BeginPlay() override;
	void UpdateMagnetTargeting();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magnet", meta = (AllowPrivateAccess = true))
	TObjectPtr<class UPhysicsHandleComponent> PhysicsHandle;
	
	TWeakObjectPtr<UPrimitiveComponent> TargetedMagnetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = true))
	EPlayerState CurrentState = EPlayerState::Normal;

public:
	virtual void Tick(float DeltaTime) override;

	void HandleInteract();
};
