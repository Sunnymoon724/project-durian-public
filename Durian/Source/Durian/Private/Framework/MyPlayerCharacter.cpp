// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MyPlayerCharacter.h"

#include "Components/PrimitiveComponent.h"
#include "Constants/GameConstants.h"
#include "CollisionShape.h"
#include "Engine/Engine.h"
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
	PhysicsHandle->LinearStiffness = 10000.0f;
	PhysicsHandle->LinearDamping = 1000.0f;
	PhysicsHandle->AngularStiffness = 5000.0f;
	PhysicsHandle->AngularDamping = 500.0f;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	
	bUseControllerRotationYaw = false;
}

void AMyPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(true);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AMyPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseMagnet();
	Super::EndPlay(EndPlayReason);
}

void AMyPlayerCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	switch (CurrentState)
	{
		case EPlayerState::Normal:
			{
				break;
			}
		case EPlayerState::MagnetTargeting:
			{
				UpdateMagnetTargeting();
				break;
			}
		case EPlayerState::MagnetControl:
			{
				UpdateMagnetControl();
				break;
			}
		case EPlayerState::Attack:
			{
				
				break;
			}
		default:
			{
				UE_LOG(LogTemp, Error, TEXT("It is not supported in EPlayerState: %s"), *UEnum::GetValueAsString(CurrentState));
				break;
			}
	}
}

void AMyPlayerCharacter::UpdateMagnetControl()
{
	if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent() || !GetController())
	{
		return;
	}

	const FVector HoldLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	const FVector TargetLocation = HoldLocation + GetActorForwardVector() * MagnetDistance;
	FVector SafeTargetLocation = TargetLocation;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagnetTargetSweep), false, this);
	if (AActor* GrabbedActor = PhysicsHandle->GetGrabbedComponent()->GetOwner())
	{
		QueryParams.AddIgnoredActor(GrabbedActor);
	}
	FHitResult SweepHit;
	const bool bBlocked = GetWorld()->SweepSingleByChannel(SweepHit, HoldLocation, TargetLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(40.0f), QueryParams);
	if (bBlocked)
	{
		SafeTargetLocation = SweepHit.Location;
	}
	PhysicsHandle->SetTargetLocation(SafeTargetLocation);
}

void AMyPlayerCharacter::UpdateMagnetTargeting()
{
	const AController* PlayerController = GetController();
	
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

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	
	UPrimitiveComponent* HitComponent = bHit ? HitResult.GetComponent() : nullptr;
	
	const bool bIsValidMagnetTarget = HitResult.GetActor() &&HitResult.GetActor()->ActorHasTag(TEXT("Magnetizable")) && HitComponent && HitComponent->IsSimulatingPhysics();
	if (bIsValidMagnetTarget)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Green, FString::Printf(TEXT("Magnet Target: %s"), *GetNameSafe(HitResult.GetActor())));
		}

		if (TargetedMagnetComponent.IsValid() && TargetedMagnetComponent.Get() != HitComponent)
		{
			TargetedMagnetComponent.Reset();
		}

		TargetedMagnetComponent = HitComponent;
		TargetedMagnetLocation = HitResult.ImpactPoint;
		
		return;
	}

	TargetedMagnetComponent.Reset();
	TargetedMagnetLocation = FVector::ZeroVector;
}

void AMyPlayerCharacter::HandleInteract()
{
	switch (CurrentState)
	{
	case EPlayerState::Normal:
		{
			SetPlayerState(EPlayerState::MagnetTargeting);
			
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Cyan, TEXT("MAGNET MODE: ON"));
			}
			
			break;
		}
	case EPlayerState::MagnetTargeting:
		{
			// 다음 단계: 유효 대상을 Activate하고 MagnetControl로 전환
			break;
		}
	default:
		{
			UE_LOG(LogTemp, Error, TEXT("It is not supported in EPlayerState: %s"), *UEnum::GetValueAsString(CurrentState));
			break;
		}
	}
}

void AMyPlayerCharacter::HandleMagnetAction()
{
	if (CurrentState == EPlayerState::MagnetControl)
	{
		ReleaseMagnet();
		return;
	}

	if (CurrentState != EPlayerState::MagnetTargeting || !PhysicsHandle || !TargetedMagnetComponent.IsValid() || !GetController())
	{
		return;
	}

	UPrimitiveComponent* Component = TargetedMagnetComponent.Get();
	if (!Component->IsSimulatingPhysics())
	{
		ReleaseMagnet();
		return;
	}

	PhysicsHandle->GrabComponentAtLocation(Component, NAME_None, TargetedMagnetLocation);
	if (PhysicsHandle->GetGrabbedComponent())
	{
		const FVector HoldLocation = GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
		MagnetDistance = FMath::Clamp(FVector::DotProduct(TargetedMagnetLocation - HoldLocation, GetActorForwardVector()), MagnetMinDistance, MagnetMaxDistance);
		CurrentState = EPlayerState::MagnetControl;
	}
}

void AMyPlayerCharacter::HandleMagnetCancel()
{
	ReleaseMagnet();
}

void AMyPlayerCharacter::HandleMagnetDistance(float AxisValue)
{
	if (!PhysicsHandle || !PhysicsHandle->GetGrabbedComponent() || FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	MagnetDistance = FMath::Clamp(MagnetDistance + AxisValue * 100.0f, MagnetMinDistance, MagnetMaxDistance);
}

void AMyPlayerCharacter::ReleaseMagnet()
{
	if (PhysicsHandle && PhysicsHandle->GetGrabbedComponent())
	{
		PhysicsHandle->ReleaseComponent();
	}

	TargetedMagnetComponent.Reset();
	TargetedMagnetLocation = FVector::ZeroVector;
	MagnetDistance = FMath::Clamp(MagnetDistance, MagnetMinDistance, MagnetMaxDistance);
	CurrentState = EPlayerState::Normal;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Yellow, TEXT("MAGNET MODE: OFF"));
	}
}

void AMyPlayerCharacter::SetPlayerState(EPlayerState NewState)
{
	if (NewState != EPlayerState::MagnetTargeting && NewState != EPlayerState::MagnetControl)
	{
		ReleaseMagnet();
		return;
	}

	CurrentState = NewState;
}
