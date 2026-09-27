// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Player/KzPlayerCharacter.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Components/DamageableComponent.h"
#include "Abilities/Core/PlayerAbilityComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
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
	Damageable = CreateDefaultSubobject<UDamageableComponent>(TEXT("Damageable"));
	Damageable->SetDestroyOwnerOnDepleted(false);
	AbilityComponent = CreateDefaultSubobject<UPlayerAbilityComponent>(TEXT("PlayerAbilityComponent"));

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

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

AKzPlayerCharacter::~AKzPlayerCharacter() = default;

void AKzPlayerCharacter::HandleInteract() const
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleInteract();
	}
}

void AKzPlayerCharacter::HandleCancel() const
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleCancel();
	}
}

void AKzPlayerCharacter::HandleAbilityUse() const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::Printf(TEXT("Ability Use : %s"), *UEnum::GetValueAsString(CurrentAbilityType)));
	}

	if (AbilityComponent)
	{
		AbilityComponent->HandleAbilityUse();
	}
}

void AKzPlayerCharacter::HandleMagnesisDistanceInput(const float AxisValue) const
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleDistance(AxisValue);
	}
}

void AKzPlayerCharacter::HandleGuard()
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleGuard();
	}
	// TODO: 현재 능력 또는 전투 상태에 따른 방어 처리를 구현한다.
	UE_LOG(LogTemp, Log, TEXT("IA_Guard received: guard is not implemented yet."));
}

float AKzPlayerCharacter::TakeDamage(const float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount > 0.0f)
	{
		if (AbilityComponent)
		{
			AbilityComponent->InterruptForDamage();
		}
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AKzPlayerCharacter::HandleAttack()
{
	if (AbilityComponent)
	{
		const FVector AttackDirection = GetController() ? GetController()->GetControlRotation().Vector().GetSafeNormal() : GetActorForwardVector();
		AbilityComponent->HandleAttack(AttackDirection);
	}
}

void AKzPlayerCharacter::HandleIceTargetAtFeet() const
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleIceTargetAtFeet();
	}
}

void AKzPlayerCharacter::HandleRemoteBombThrow() const
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleRemoteBombThrow();
	}
}

void AKzPlayerCharacter::SetAbility(const EAbilityType NewAbility)
{
	if (AbilityComponent)
	{
		AbilityComponent->SetAbility(NewAbility);
	}
}

void AKzPlayerCharacter::SetPlayerState(const EPlayerState NewState)
{
	if (NewState == EPlayerState::Attack)
	{
		if (AbilityComponent)
		{
			AbilityComponent->InterruptForStateChange();
		}
	}
	CurrentState = NewState;
}
