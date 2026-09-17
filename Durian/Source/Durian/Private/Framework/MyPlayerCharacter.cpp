// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MyPlayerCharacter.h"

#include "Abilities/AbilityEffectComponent.h"
#include "Abilities/MagnetAbility.h"
#include "Abilities/TimeLockAbility.h"
#include "Abilities/RemoteBombAbility.h"
#include "Abilities/IceMakerAbility.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

AMyPlayerCharacter::AMyPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	PhysicsHandle->LinearStiffness = 10000.0f;
	PhysicsHandle->LinearDamping = 1000.0f;
	PhysicsHandle->AngularStiffness = 5000.0f;
	PhysicsHandle->AngularDamping = 500.0f;

	AbilityEffect = CreateDefaultSubobject<UAbilityEffectComponent>(TEXT("AbilityEffect"));
	MagnetAbility = MakeUnique<FMagnetAbility>(this);
	TimeLockAbility = MakeUnique<FTimeLockAbility>(this);
	RemoteBombAbility = MakeUnique<FRemoteBombAbility>(this);
	IceMakerAbility = MakeUnique<FIceMakerAbility>(this);
	CurrentAbility = MagnetAbility.Get();

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void AMyPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	TArray<UPrimitiveComponent*> PlayerPrimitiveComponents;
	GetComponents(PlayerPrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PlayerPrimitiveComponents)
	{
		if (IsValid(PrimitiveComponent))
		{
			PrimitiveComponent->SetRenderCustomDepth(true);
			PrimitiveComponent->SetCustomDepthStencilValue(3);
		}
	}

	SetActorTickEnabled(true);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AMyPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleCancel();
	}

	Super::EndPlay(EndPlayReason);
}

AMyPlayerCharacter::~AMyPlayerCharacter() = default;

void AMyPlayerCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentAbility)
	{
		CurrentAbility->Tick(DeltaTime);
	}
}

void AMyPlayerCharacter::HandleInteract() const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleInteract();
	}
}

void AMyPlayerCharacter::HandleCancel() const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleCancel();
	}
}

void AMyPlayerCharacter::HandleAbilityUse() const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleAbilityUse();
	}
}

void AMyPlayerCharacter::HandleMagnetDistanceInput(const float AxisValue) const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleDistance(AxisValue);
	}
}

void AMyPlayerCharacter::HandleGuard() const
{
	// TODO: 현재 능력 또는 전투 상태에 따른 방어 처리를 구현한다.
	UE_LOG(LogTemp, Log, TEXT("IA_Guard received: guard is not implemented yet."));
}

void AMyPlayerCharacter::HandleIceTargetAtFeet() const
{
	if (IceMakerAbility)
	{
		IceMakerAbility->HandleTargetAtFeet();
	}
}

void AMyPlayerCharacter::HandleBombThrow() const
{
	if (RemoteBombAbility)
	{
		RemoteBombAbility->HandleBombThrow();
	}
}

void AMyPlayerCharacter::SetAbility(const EAbilityType NewAbility)
{
	switch (NewAbility)
	{
	case EAbilityType::Magnet:
		CurrentAbility = MagnetAbility.Get();
		break;
	case EAbilityType::Ice:
		CurrentAbility = IceMakerAbility.Get();
		break;
	case EAbilityType::Stasis:
		CurrentAbility = TimeLockAbility.Get();
		break;
	case EAbilityType::Bomb:
		CurrentAbility = RemoteBombAbility.Get();
		break;
	default:
		CurrentAbility = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(NewAbility));
		break;
	}
}

void AMyPlayerCharacter::SetPlayerState(const EPlayerState NewState)
{
	CurrentState = NewState;
}
