// Fill out your copyright notice in the Description page of Project Settings.


// ReSharper disable All
#include "Framework/Player/KzPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

AKzPlayerController::AKzPlayerController()
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
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Guard.IA_Guard"), GuardAction);
	
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_AbilityUse.IA_AbilityUse"), AbilityUseAction);
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Interact.IA_Interact"),InteractAction);
	
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Cancel.IA_Cancel"), CancelAction);

	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_Menu.IA_Menu"), MenuAction);
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_AbilityWheel.IA_AbilityWheel"), AbilityWheelAction);

	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_IceTargetAtFeet.IA_IceTargetAtFeet"), IceTargetAtFeetAction);
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_BombThrow.IA_BombThrow"), RemoteBombThrowAction);
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_RestartLevel.IA_RestartLevel"), RestartLevelAction);

	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_MagnesisDistance.IA_MagnesisDistance"), MagnesisDistanceAction);
}

void AKzPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedCharacter = Cast<AKzPlayerCharacter>(GetCharacter());
}

void AKzPlayerController::BeginPlay()
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

void AKzPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AKzPlayerController::OnLook);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AKzPlayerController::OnJumpStarted);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AKzPlayerController::OnJumpCompleted);
		}

		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AKzPlayerController::OnMove);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AKzPlayerController::OnInteract);
		}

		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AKzPlayerController::OnAttack);
		}

		if (AbilityUseAction)
		{
			EnhancedInputComponent->BindAction(AbilityUseAction, ETriggerEvent::Started, this, &AKzPlayerController::OnAbilityUse);
		}

		if (CancelAction)
		{
			EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Started, this, &AKzPlayerController::OnCancel);
		}

		if (MenuAction)
		{
			EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Started, this, &AKzPlayerController::OnMenu);
		}

		if (AbilityWheelAction)
		{
			EnhancedInputComponent->BindAction(AbilityWheelAction, ETriggerEvent::Started, this, &AKzPlayerController::OnAbilityWheel);
			EnhancedInputComponent->BindAction(AbilityWheelAction, ETriggerEvent::Completed, this, &AKzPlayerController::OnAbilityWheelCompleted);
		}

		if (GuardAction)
		{
			EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, this, &AKzPlayerController::OnGuard);
		}

		if (IceTargetAtFeetAction)
		{
			EnhancedInputComponent->BindAction(IceTargetAtFeetAction, ETriggerEvent::Started, this, &AKzPlayerController::OnIceTargetAtFeet);
		}

		if (RemoteBombThrowAction)
		{
			EnhancedInputComponent->BindAction(RemoteBombThrowAction, ETriggerEvent::Started, this, &AKzPlayerController::OnRemoteBombThrow);
		}

		if (RestartLevelAction)
		{
			EnhancedInputComponent->BindAction(RestartLevelAction, ETriggerEvent::Started, this, &AKzPlayerController::OnRestartLevel);
		}

		if (MagnesisDistanceAction)
		{
			EnhancedInputComponent->BindAction(MagnesisDistanceAction, ETriggerEvent::Triggered, this, &AKzPlayerController::OnMagnesisDistance);
		}
	}
}

void AKzPlayerController::OnMove(const FInputActionValue& Value)
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();

	FVector CameraLocation;
	FRotator CameraRotation;

	GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FRotator YawRotation(0, CameraRotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector DesiredDirection = (ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X).GetSafeNormal();

	if (!DesiredDirection.IsNearlyZero())
	{
		ControlledCharacter->AddMovementInput(DesiredDirection, 1.0f);

		const FRotator MovementRotation(0.f, DesiredDirection.Rotation().Yaw, 0.f);

		ControlledCharacter->SetActorRotation(MovementRotation);
	}
}

void AKzPlayerController::OnLook(const FInputActionValue& Value)
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	ControlledCharacter->AddControllerYawInput(LookAxisVector.X);
	ControlledCharacter->AddControllerPitchInput(LookAxisVector.Y);
}

void AKzPlayerController::OnMoveForward(const float Value)
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

	ControlledCharacter->AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void AKzPlayerController::OnMoveRight(const float Value)
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

	ControlledCharacter->AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void AKzPlayerController::OnTurn(const float Value)
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->AddControllerYawInput(Value);
}

void AKzPlayerController::OnLookUp(const float Value)
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->AddControllerPitchInput(Value);
}

void AKzPlayerController::OnJumpStarted()
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->Jump();
}

void AKzPlayerController::OnJumpCompleted()
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->StopJumping();
}

void AKzPlayerController::OnInteract()
{
	if (AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter())
	{
		ControlledCharacter->HandleInteract();
	}
}

void AKzPlayerController::OnCancel()
{
	if (AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter())
	{
		ControlledCharacter->HandleCancel();
	}
}

void AKzPlayerController::OnMenu()
{
	MenuRequested();
}

void AKzPlayerController::OnAbilityWheel()
{
	AbilitySelectionRequested();
}

void AKzPlayerController::OnAbilityWheelCompleted()
{
	// TODO: 강조된 능력을 CurrentAbility로 확정한다.
	UE_LOG(LogTemp, Log, TEXT("IA_AbilityWheel completed: ability selection apply is not implemented yet."));
}

void AKzPlayerController::RestartCurrentLevel()
{
	if (const UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, FName(*World->GetName()));
	}
}

void AKzPlayerController::OnAbilityUse()
{
	if (AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter())
	{
		ControlledCharacter->HandleAbilityUse();
	}
}

void AKzPlayerController::OnAttack()
{
	AttackRequested();
}

void AKzPlayerController::OnGuard()
{
	if (AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter())
	{
		ControlledCharacter->HandleGuard();
	}
}

void AKzPlayerController::OnIceTargetAtFeet()
{
	if (AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter())
	{
		ControlledCharacter->HandleIceTargetAtFeet();
	}
}

void AKzPlayerController::OnRemoteBombThrow()
{
	if (AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter())
	{
		ControlledCharacter->HandleRemoteBombThrow();
	}
}

void AKzPlayerController::OnRestartLevel()
{
	UE_LOG(LogTemp, Log, TEXT("IA_RestartLevel pressed."));
	RestartCurrentLevel();
}

void AKzPlayerController::OnMagnesisDistance(const FInputActionValue& Value)
{
	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();
	
	if (!ControlledCharacter)
	{
		return;
	}

	ControlledCharacter->HandleMagnesisDistanceInput(Value.Get<float>());
}

AKzPlayerCharacter* AKzPlayerController::GetControlledCharacter()
{
	AKzPlayerCharacter* ControlledCharacter = Cast<AKzPlayerCharacter>(GetPawn());

	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Look called but current Pawn is not AKzPlayerCharacter: %s"), *GetNameSafe(GetPawn()));

		return nullptr;
	}

	return  ControlledCharacter;
}
