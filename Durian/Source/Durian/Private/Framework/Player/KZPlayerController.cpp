// Fill out your copyright notice in the Description page of Project Settings.


// ReSharper disable All
#include "Framework/Player/KzPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "UI/KzAbilityWheelWidget.h"
#include "UI/KzHudWidget.h"
#include "Engine/Engine.h"

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
	SetObjectPtrImpl(TEXT("/Game/Inputs/IA_MagnesisDistance.IA_MagnesisDistance"), MagnesisDistanceAction);

	static ConstructorHelpers::FClassFinder<UKzAbilityWheelWidget> AbilityWheelWidgetAsset(TEXT("/Game/_BP/Widgets/WBP_AbilityWheel"));

	if (AbilityWheelWidgetAsset.Succeeded())
	{
		AbilityWheelWidgetClass = AbilityWheelWidgetAsset.Class;
	}

	static ConstructorHelpers::FClassFinder<UKzHudWidget> HudWidgetAsset(TEXT("/Game/_BP/Widgets/WBP_Hud"));

	if (HudWidgetAsset.Succeeded())
	{
		HudWidgetClass = HudWidgetAsset.Class;
	}

	UE_LOG(LogTemp, Warning, TEXT("Ability Wheel class load: %s"), *GetNameSafe(AbilityWheelWidgetClass));
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

	if (AbilityWheelWidgetClass)
	{
		AbilityWheelWidget = CreateWidget<UKzAbilityWheelWidget>(this, AbilityWheelWidgetClass);

		if (AbilityWheelWidget)
		{
			AbilityWheelWidget->OnAbilityConfirmed.AddDynamic(this, &AKzPlayerController::HandleAbilityWheelConfirmed);
			AbilityWheelWidget->AddToPlayerScreen();
		}
	}

	if (HudWidgetClass)
	{
		HudWidget = CreateWidget<UKzHudWidget>(this, HudWidgetClass);

		if (HudWidget)
		{
			HudWidget->AddToPlayerScreen();
			HudWidget->SetCurrentAbility(CachedCharacter ? CachedCharacter->GetCurrentAbilityType() : EAbilityType::Magnesis);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("PlayerController BeginPlay: AbilityWheelAction=%s LookAction=%s MappingContext=%s WidgetClass=%s Widget=%s"),
		*GetNameSafe(AbilityWheelAction),
		*GetNameSafe(LookAction),
		*GetNameSafe(CharacterMappingContext),
		*GetNameSafe(AbilityWheelWidgetClass),
		*GetNameSafe(AbilityWheelWidget));

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
	if (AbilityWheelWidget && AbilityWheelWidget->IsWheelOpen())
	{
		const FVector2D LookInput = Value.Get<FVector2D>();

		UE_LOG(LogTemp, Warning, TEXT("Ability Wheel Look input: X=%.3f Y=%.3f"), LookInput.X, LookInput.Y);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(200, 0.25f, FColor::Cyan,
				FString::Printf(TEXT("Ability Wheel Look: X %.2f Y %.2f"), LookInput.X, LookInput.Y));
		}

		AbilityWheelWidget->AddSelectionDirection(LookInput);
		return;
	}

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
	UE_LOG(LogTemp, Warning, TEXT("Ability Wheel started"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(201, 1.0f, FColor::Yellow, TEXT("Ability Wheel Started"));
	}

	AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter();

	if (!AbilityWheelWidget || !ControlledCharacter)
	{
		AbilitySelectionRequested();
		return;
	}

	AbilityWheelWidget->SetHighlightedAbility(ControlledCharacter->GetCurrentAbilityType());
	AbilityWheelWidget->OpenWheel();
}

void AKzPlayerController::OnAbilityWheelCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("Ability Wheel completed"));

	if (AbilityWheelWidget)
	{
		AbilityWheelWidget->ConfirmSelection();
	}
}

void AKzPlayerController::HandleAbilityWheelConfirmed(const EAbilityType AbilityType)
{
	if (AbilityType == EAbilityType::None)
	{
		return;
	}

	if (AKzPlayerCharacter* ControlledCharacter = GetControlledCharacter())
	{
		ControlledCharacter->SetAbility(AbilityType);

		if (HudWidget)
		{
			HudWidget->SetCurrentAbility(AbilityType);
		}
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
