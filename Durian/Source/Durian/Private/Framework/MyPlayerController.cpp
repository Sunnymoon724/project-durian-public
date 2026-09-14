// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MyPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Framework/MyPlayerCharacter.h"

AMyPlayerController::AMyPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> CharacterMappingContextAsset(TEXT("/Game/Inputs/IMC_Character.IMC_Character"));

	if (CharacterMappingContextAsset.Succeeded())
	{
		CharacterMappingContext = CharacterMappingContextAsset.Object;
	}

	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Look.IA_Look"), LookAction);

	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Jump.IA_Jump"), JumpAction);
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Move.IA_Move"), MoveAction);

	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Attack.IA_Attack"), AttackAction);
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Interact.IA_Interact"),InteractAction);
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Cancel.IA_Cancel"), CancelAction);

	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_RestartLevel.IA_RestartLevel"), RestartLevelAction);
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedCharacter = Cast<AMyPlayerCharacter>(GetCharacter());
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	const  FInputModeGameOnly InputMode;

	SetInputMode(InputMode);
	EnableInput(this);

	if (InputComponent)
	{
		InputComponent->Activate(true);
	}

	bShowMouseCursor = false;

	if (CharacterMappingContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			SubSystem->AddMappingContext(CharacterMappingContext, 100);
		}
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyPlayerController::LookImpl);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyPlayerController::JumpImpl);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMyPlayerController::StopJumpingImpl);
		}

		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPlayerController::MoveImpl);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMyPlayerController::OnInteract);
		}
	}
}

void AMyPlayerController::MoveImpl(const FInputActionValue& Value)
{
	AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	ControlledCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
	ControlledCharacter->AddMovementInput(RightDirection, MovementVector.X);
}

void AMyPlayerController::LookImpl(const FInputActionValue& Value)
{
	AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	ControlledCharacter->AddControllerYawInput(LookAxisVector.X);
	ControlledCharacter->AddControllerPitchInput(LookAxisVector.Y);
}

void AMyPlayerController::MoveForwardImpl(const float Value)
{
	AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

	ControlledCharacter->AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void AMyPlayerController::MoveRightImpl(const float Value)
{
	AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

	ControlledCharacter->AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void AMyPlayerController::TurnImpl(const float Value)
{
	AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->AddControllerYawInput(Value);
}

void AMyPlayerController::LookUpImpl(const float Value)
{
	AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->AddControllerPitchInput(Value);
}

void AMyPlayerController::JumpImpl()
{
	AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->Jump();
}

void AMyPlayerController::StopJumpingImpl()
{
	AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->StopJumping();
}

void AMyPlayerController::OnInteract()
{
	if (AMyPlayerCharacter* ControlledCharacter = GetControlledCharacter())
	{
		ControlledCharacter->HandleInteract();
	}
}

void AMyPlayerController::OnMagnetCancel()
{
	
}

void AMyPlayerController::OnMagnetRestart()
{
	
}

AMyPlayerCharacter* AMyPlayerController::GetControlledCharacter()
{
	AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn());

	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Look called but current Pawn is not AMyPlayerCharacter: %s"), *GetNameSafe(GetPawn()));

		return nullptr;
	}

	return  ControlledCharacter;
}
