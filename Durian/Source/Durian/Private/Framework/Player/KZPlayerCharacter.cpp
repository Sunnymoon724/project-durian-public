// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Player/KzPlayerCharacter.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Magnesis/MagnesisAbility.h"
#include "Abilities/Stasis/StasisAbility.h"
#include "Abilities/RemoteBomb/RemoteBombAbility.h"
#include "Abilities/Cryonis/CryonisAbility.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

AKzPlayerCharacter::AKzPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	PhysicsHandle->LinearStiffness = 10000.0f;
	PhysicsHandle->LinearDamping = 1000.0f;
	PhysicsHandle->AngularStiffness = 5000.0f;
	PhysicsHandle->AngularDamping = 500.0f;

	AbilityEffect = CreateDefaultSubobject<UAbilityEffectComponent>(TEXT("AbilityEffect"));

	MagnesisAbility = MakeUnique<FMagnesisAbility>(this);
	StasisAbility = MakeUnique<FStasisAbility>(this);
	RemoteBombAbility = MakeUnique<FRemoteBombAbility>(this);
	CryonisAbility = MakeUnique<FCryonisAbility>(this);

	CurrentAbility = MagnesisAbility.Get();

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void AKzPlayerCharacter::BeginPlay()
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

void AKzPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleCancel();
	}

	Super::EndPlay(EndPlayReason);
}

AKzPlayerCharacter::~AKzPlayerCharacter() = default;

void AKzPlayerCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentAbility)
	{
		CurrentAbility->Tick(DeltaTime);
	}
}

void AKzPlayerCharacter::HandleInteract() const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleInteract();
	}
}

void AKzPlayerCharacter::HandleCancel() const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleCancel();
	}
}

void AKzPlayerCharacter::HandleAbilityUse() const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Ability Use : %s"), *UEnum::GetValueAsString(CurrentAbilityType)));
	}

	if (CurrentAbility)
	{
		CurrentAbility->HandleAbilityUse();
	}
}

void AKzPlayerCharacter::HandleMagnesisDistanceInput(const float AxisValue) const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleDistance(AxisValue);
	}
}

void AKzPlayerCharacter::HandleGuard() const
{
	// TODO: 현재 능력 또는 전투 상태에 따른 방어 처리를 구현한다.
	UE_LOG(LogTemp, Log, TEXT("IA_Guard received: guard is not implemented yet."));
}

void AKzPlayerCharacter::HandleIceTargetAtFeet() const
{
	if (CryonisAbility)
	{
		CryonisAbility->HandleTargetAtFeet();
	}
}

void AKzPlayerCharacter::HandleRemoteBombThrow() const
{
	if (RemoteBombAbility)
	{
		RemoteBombAbility->HandleRemoteBombThrow();
	}
}

void AKzPlayerCharacter::SetAbility(const EAbilityType NewAbility)
{
	FAbility* PreviousAbility = CurrentAbility;
	CurrentAbilityType = NewAbility;

	switch (NewAbility)
	{
	case EAbilityType::Magnesis:
		CurrentAbility = MagnesisAbility.Get();
		break;
	case EAbilityType::Cryonis:
		CurrentAbility = CryonisAbility.Get();
		break;
	case EAbilityType::Stasis:
		CurrentAbility = StasisAbility.Get();
		break;
	case EAbilityType::RemoteBombSphere:
	case EAbilityType::RemoteBombCube:
		CurrentAbility = RemoteBombAbility.Get();
		break;
	default:
		CurrentAbility = nullptr;
		UE_LOG(LogTemp, Log, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(NewAbility));
		break;
	}

	if (PreviousAbility && PreviousAbility != CurrentAbility)
	{
		PreviousAbility->HandleCancel();
	}
}

void AKzPlayerCharacter::SetPlayerState(const EPlayerState NewState)
{
	CurrentState = NewState;
}
