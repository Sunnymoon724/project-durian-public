// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionDescriptor;
class AMyPlayerCharacter;

/**
 * 
 */


UCLASS()
class DURIAN_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMyPlayerController();
	
private:
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputMappingContext> CharacterMappingContext;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> AttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> CancelAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> RestartLevelAction;

	UPROPERTY();
	TObjectPtr<AMyPlayerCharacter> CachedCharacter;

	template <typename TType>
	void SetObjectPtrImpl(const FString& AssetName, TObjectPtr<TType>& ObjectPtr)
	{
		ConstructorHelpers::FObjectFinder<TType> LoadedAsset(*AssetName);

		if (LoadedAsset.Succeeded())
		{
			ObjectPtr = LoadedAsset.Object;
		}
	}
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
	
private:
	void LookImpl(const FInputActionValue& Value);
	void LookUpImpl(const float Value);

	void MoveImpl(const FInputActionValue& Value);
	void MoveForwardImpl(const float Value);
	void MoveRightImpl(const float Value);
	void TurnImpl(const float Value);

	void JumpImpl();
	void StopJumpingImpl();
	
	void OnInteract();
	void OnMagnetCancel();
	void OnMagnetRestart();
	
	AMyPlayerCharacter* GetControlledCharacter();
};
