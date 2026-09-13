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
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<UInputMappingContext> CharacterMappingContext;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Input",meta=(AllowPrivateAccess=true))
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY();
	TObjectPtr<AMyPlayerCharacter> CachedCharacter;
	
	template <typename TType>
	void setObjectPtr(const FString& AssetName, TObjectPtr<TType>& ObjectPtr)
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
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	
	void Jump();
	void StopJumping();
};
