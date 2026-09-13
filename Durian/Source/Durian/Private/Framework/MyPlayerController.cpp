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
	
	setObjectPtr(TEXT("/Game/Inputs/IA_Jump.IA_Jump"), JumpAction);
	setObjectPtr(TEXT("/Game/Inputs/IA_Move.IA_Move"), MoveAction);
	setObjectPtr(TEXT("/Game/Inputs/IA_Look.IA_Look"), LookAction);
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	CachedCharacter = Cast<AMyPlayerCharacter>(GetCharacter());
	UE_LOG(LogTemp, Warning, TEXT("MyPlayerController OnPossess: Pawn=%s, CharacterCast=%s"), *GetNameSafe(InPawn), *GetNameSafe(CachedCharacter));
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("MyPlayerController BeginPlay: MappingContext=%s, LocalPlayer=%s"), *GetNameSafe(CharacterMappingContext), *GetNameSafe(GetLocalPlayer()));

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
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
		UE_LOG(LogTemp, Warning, TEXT("MyPlayerController SetupInputComponent: EnhancedInputComponent OK"));
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyPlayerController::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMyPlayerController::StopJumping);
		}
		
	}

	// DefaultInput.ini에 등록된 축 매핑을 사용한다. 현재 프로젝트에서는 Enhanced Input
	// 액션 이벤트가 PIE에서 전달되지 않아, 이 경로를 단일 이동 입력 경로로 유지한다.
	InputComponent->BindAxis(TEXT("Move Forward / Backward"), this, &AMyPlayerController::MoveForward);
	InputComponent->BindAxis(TEXT("Move Right / Left"), this, &AMyPlayerController::MoveRight);
	InputComponent->BindAxis(TEXT("Turn Right / Left Mouse"), this, &AMyPlayerController::Turn);
	InputComponent->BindAxis(TEXT("Turn Right / Left Gamepad"), this, &AMyPlayerController::Turn);
	InputComponent->BindAxis(TEXT("Look Up / Down Mouse"), this, &AMyPlayerController::LookUp);
	InputComponent->BindAxis(TEXT("Look Up / Down Gamepad"), this, &AMyPlayerController::LookUp);
}

void AMyPlayerController::Move(const FInputActionValue& Value)
{
	AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn());
	if (!ControlledCharacter)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Move input received"));

	FVector2D MovementVector = Value.Get<FVector2D>();
	
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	ControlledCharacter->AddMovementInput(ForwardDirection, MovementVector.Y);
	ControlledCharacter->AddMovementInput(RightDirection, MovementVector.X);
}

void AMyPlayerController::Look(const FInputActionValue& Value)
{
	AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn());
	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("Look called but current Pawn is not AMyPlayerCharacter: %s"), *GetNameSafe(GetPawn()));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Look input received"));

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	ControlledCharacter->AddControllerYawInput(LookAxisVector.X);
	ControlledCharacter->AddControllerPitchInput(LookAxisVector.Y);
}

void AMyPlayerController::MoveForward(float Value)
{
	if (AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn()))
	{
		const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
		ControlledCharacter->AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
	}
}

void AMyPlayerController::MoveRight(float Value)
{
	if (AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn()))
	{
		const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
		ControlledCharacter->AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
	}
}

void AMyPlayerController::Turn(float Value)
{
	if (AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn()))
	{
		ControlledCharacter->AddControllerYawInput(Value);
	}
}

void AMyPlayerController::LookUp(float Value)
{
	if (AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn()))
	{
		ControlledCharacter->AddControllerPitchInput(Value);
	}
}

void AMyPlayerController::Jump()
{
	if (AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn()))
	{
		ControlledCharacter->Jump();
	}
}

void AMyPlayerController::StopJumping()
{
	if (AMyPlayerCharacter* ControlledCharacter = Cast<AMyPlayerCharacter>(GetPawn()))
	{
		ControlledCharacter->StopJumping();
	}
}
