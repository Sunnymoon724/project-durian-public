// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZeldaCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "Engine/Engine.h"
#include "Durian.h"

AZeldaCharacter::AZeldaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AZeldaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AZeldaCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AZeldaCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AZeldaCharacter::Look);
	}
	else
	{
		UE_LOG(LogDurian, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// Temporary keyboard bindings for the shrine ability prototype. These avoid
	// requiring new Input Action assets while the selection flow is validated.
	PlayerInputComponent->BindKey(EKeys::One, IE_Pressed, this, &AZeldaCharacter::SelectMagnetAbility);
	PlayerInputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AZeldaCharacter::SelectIceAbility);
	PlayerInputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AZeldaCharacter::SelectStasisAbility);
	PlayerInputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AZeldaCharacter::SelectRemoteBombAbility);
	PlayerInputComponent->BindKey(EKeys::Q, IE_Pressed, this, &AZeldaCharacter::ClearSelectedAbility);
}

void AZeldaCharacter::SelectMagnetAbility()
{
	SetSelectedAbility(EPlayerAbility::Magnet);
}

void AZeldaCharacter::SelectIceAbility()
{
	SetSelectedAbility(EPlayerAbility::Ice);
}

void AZeldaCharacter::SelectStasisAbility()
{
	SetSelectedAbility(EPlayerAbility::Stasis);
}

void AZeldaCharacter::SelectRemoteBombAbility()
{
	SetSelectedAbility(EPlayerAbility::RemoteBomb);
}

void AZeldaCharacter::ClearSelectedAbility()
{
	SetSelectedAbility(EPlayerAbility::None);
}

void AZeldaCharacter::SetSelectedAbility(const EPlayerAbility NewAbility)
{
	if (SelectedAbility == NewAbility)
	{
		return;
	}

	SelectedAbility = NewAbility;
	OnSelectedAbilityChanged(SelectedAbility);

	const UEnum* AbilityEnum = StaticEnum<EPlayerAbility>();
	const FString AbilityName = AbilityEnum
		? AbilityEnum->GetDisplayNameTextByValue(static_cast<int64>(SelectedAbility)).ToString()
		: TEXT("Unknown");

	UE_LOG(LogDurian, Log, TEXT("Selected player ability: %s"), *AbilityName);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			2.0f,
			FColor::Cyan,
			FString::Printf(TEXT("Ability: %s"), *AbilityName));
	}
}

void AZeldaCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AZeldaCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AZeldaCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AZeldaCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AZeldaCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AZeldaCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AZeldaCharacter::Interact()
{
	if (SelectedAbility == EPlayerAbility::Magnet)
	{
		UE_LOG(LogDurian, Log, TEXT("Magnet ability selected"));
		return;
	}

	if (SelectedAbility == EPlayerAbility::Ice)
	{
		UE_LOG(LogDurian, Log, TEXT("Ice ability selected"));
		return;
	}

	if (SelectedAbility == EPlayerAbility::Stasis)
	{
		UE_LOG(LogDurian, Log, TEXT("Stasis ability selected"));
		return;
	}

	if (SelectedAbility == EPlayerAbility::RemoteBomb)
	{
		UE_LOG(LogDurian, Log, TEXT("RemoteBomb ability selected"));
		return;
	}
}
