// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MyPlayerCharacter.h"

#include "Abilities/MagnetTargetComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Constants/GameConstants.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

// Sets default values
AMyPlayerCharacter::AMyPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AMyPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AMyPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EPlayerState::MagnetTargeting)
	{
		UpdateMagnetTargeting();
	}
}

void AMyPlayerCharacter::UpdateMagnetTargeting()
{
	AController* PlayerController = GetController();
	if (!PlayerController)
	{
		TargetedMagnetComponent.Reset();
		return;
	}

	FVector TraceStart;
	FRotator ViewRotation;
	PlayerController->GetPlayerViewPoint(TraceStart, ViewRotation);

	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * Constants::MagnetTargetRange;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	UPrimitiveComponent* HitComponent = bHit ? HitResult.GetComponent() : nullptr;
	const bool bIsValidMagnetTarget =
		HitResult.GetActor() &&
		HitResult.GetActor()->FindComponentByClass<UMagnetTargetComponent>() &&
		HitComponent &&
		HitComponent->IsSimulatingPhysics();

	DrawDebugLine(
		GetWorld(),
		TraceStart,
		bHit ? HitResult.ImpactPoint : TraceEnd,
		bIsValidMagnetTarget ? FColor::Green : FColor::Red,
		false,
		0.0f,
		0,
		1.0f
	);

	if (bIsValidMagnetTarget)
	{
		TargetedMagnetComponent = HitComponent;
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 20.0f, 12, FColor::Green, false, 0.0f);
		return;
	}

	TargetedMagnetComponent.Reset();
}

void AMyPlayerCharacter::HandleInteract()
{
	switch (CurrentState)
	{
	case EPlayerState::Normal:
		{
			CurrentState = EPlayerState::MagnetTargeting;
			break;
		}
	case EPlayerState::MagnetTargeting:
		{
			// 다음 단계: 유효 대상을 Activate하고 MagnetControl로 전환
			break;
		}
	default:
		{
			break;
		}
	}
}
