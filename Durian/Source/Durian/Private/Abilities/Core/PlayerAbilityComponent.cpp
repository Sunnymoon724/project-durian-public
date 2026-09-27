#include "Abilities/Core/PlayerAbilityComponent.h"

#include "Abilities/Cryonis/CryonisAbility.h"
#include "Abilities/Magnesis/MagnesisAbility.h"
#include "Abilities/RemoteBomb/RemoteBombAbility.h"
#include "Abilities/Stasis/StasisAbility.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Constants/GameConstantsDataAsset.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerAbilityComponent::UPlayerAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

UPlayerAbilityComponent::~UPlayerAbilityComponent() = default;

void UPlayerAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<AKzPlayerCharacter>(GetOwner());
	if (!Character)
	{
		SetComponentTickEnabled(false);
		return;
	}
	MagnesisAbility = MakeUnique<FMagnesisAbility>(Character);
	StasisAbility = MakeUnique<FStasisAbility>(Character);
	RemoteBombAbility = MakeUnique<FRemoteBombAbility>(Character, RemoteBombSphereClass, RemoteBombCubeClass);
	CryonisAbility = MakeUnique<FCryonisAbility>(Character);
	CurrentAbility = MagnesisAbility.Get();
}

void UPlayerAbilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleCancel();
	}
	if (StasisAbility)
	{
		StasisAbility->AbortForEndPlay();
	}
	if (RemoteBombAbility)
	{
		RemoteBombAbility->AbortForEndPlay();
	}
	Super::EndPlay(EndPlayReason);
}

void UPlayerAbilityComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (StasisAbility)
	{
		StasisAbility->TickPersistent(DeltaTime);
	}
	if (CurrentAbility)
	{
		CurrentAbility->Tick(DeltaTime);
	}
}

void UPlayerAbilityComponent::SetAbility(const EAbilityType NewAbility)
{
	if (!Character)
	{
		return;
	}
	FAbility* PreviousAbility = CurrentAbility;
	const EAbilityType PreviousAbilityType = Character->GetCurrentAbilityType();
	Character->CurrentAbilityType = NewAbility;
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
	if (PreviousAbility == RemoteBombAbility.Get() && PreviousAbilityType != NewAbility)
	{
		RemoteBombAbility->HandleAbilityDeselected();
	}
	else if (PreviousAbility && PreviousAbility != CurrentAbility)
	{
		PreviousAbility->HandleCancel();
		if (PreviousAbility == StasisAbility.Get() && StasisAbility->IsStasisActive() && Character->GetCurrentState() == EPlayerState::StasisActive)
		{
			Character->SetPlayerState(EPlayerState::Normal);
		}
	}
	Character->OnAbilityChanged.Broadcast(NewAbility);
}

void UPlayerAbilityComponent::HandleInteract()
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleInteract();
	}
}

void UPlayerAbilityComponent::HandleCancel()
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleCancel();
	}
}

void UPlayerAbilityComponent::HandleAbilityUse()
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleAbilityUse();
	}
}

void UPlayerAbilityComponent::HandleDistance(const float AxisValue)
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleDistance(AxisValue);
	}
}

void UPlayerAbilityComponent::HandleGuard()
{
	InterruptTargeting();
	DropHeldBomb();
}

void UPlayerAbilityComponent::HandleAttack(const FVector& AttackDirection)
{
	InterruptTargeting();
	DropHeldBomb();
	if (!StasisAbility || !StasisAbility->IsStasisActive() || !Character || !Character->GetWorld())
	{
		return;
	}
	const UGameConstantsDataAsset* Constants = UGameConstantsDataAsset::Get();
	const FVector TraceStart = Character->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FVector TraceEnd = TraceStart + AttackDirection * Constants->StasisMeleeRange;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(StasisMeleeAttack), false, Character);
	QueryParams.AddIgnoredActor(Character);
	FHitResult Hit;
	if (Character->GetWorld()->SweepSingleByChannel(Hit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Constants->StasisMeleeRadius), QueryParams))
	{
		StasisAbility->HandleAttackHit(Hit, AttackDirection);
	}
}

void UPlayerAbilityComponent::HandleIceTargetAtFeet()
{
	if (CryonisAbility)
	{
		CryonisAbility->HandleTargetAtFeet();
	}
}

void UPlayerAbilityComponent::HandleRemoteBombThrow()
{
	if (RemoteBombAbility)
	{
		RemoteBombAbility->HandleRemoteBombThrow();
	}
}

void UPlayerAbilityComponent::InterruptForDamage()
{
	InterruptTargeting();
	DropHeldBomb();
}

void UPlayerAbilityComponent::InterruptForStateChange()
{
	InterruptTargeting();
}

void UPlayerAbilityComponent::InterruptTargeting()
{
	if (!Character)
	{
		return;
	}
	const EPlayerState State = Character->GetCurrentState();
	if (MagnesisAbility && (State == EPlayerState::MagnesisTargeting || State == EPlayerState::MagnesisHolding))
	{
		MagnesisAbility->Release();
	}
	if (CryonisAbility && State == EPlayerState::IceTargeting)
	{
		CryonisAbility->HandleCancel();
	}
	if (StasisAbility && State == EPlayerState::StasisTargeting)
	{
		StasisAbility->HandleCancel();
	}
}

void UPlayerAbilityComponent::DropHeldBomb()
{
	if (RemoteBombAbility)
	{
		RemoteBombAbility->DropHeldBomb();
	}
}

float UPlayerAbilityComponent::GetRemoteBombCooldown(const ERemoteBombShape Shape) const
{
	return RemoteBombAbility ? RemoteBombAbility->GetCooldownRemaining(Shape) : 0.0f;
}

bool UPlayerAbilityComponent::HasRemoteBomb(const ERemoteBombShape Shape) const
{
	return RemoteBombAbility && RemoteBombAbility->HasBomb(Shape);
}

bool UPlayerAbilityComponent::IsRemoteBombInstalled(const ERemoteBombShape Shape) const
{
	return RemoteBombAbility && RemoteBombAbility->IsBombInstalled(Shape);
}

bool UPlayerAbilityComponent::IsHoldingRemoteBomb() const
{
	return RemoteBombAbility && RemoteBombAbility->IsHoldingBomb();
}

float UPlayerAbilityComponent::GetStasisRemainingTime() const
{
	return StasisAbility ? StasisAbility->GetRemainingTime() : 0.0f;
}

float UPlayerAbilityComponent::GetStasisCooldownRemaining() const
{
	return StasisAbility ? StasisAbility->GetCooldownRemaining() : 0.0f;
}

FVector UPlayerAbilityComponent::GetStasisAccumulatedImpulse() const
{
	return StasisAbility ? StasisAbility->GetAccumulatedImpulse() : FVector::ZeroVector;
}

bool UPlayerAbilityComponent::IsStasisActive() const
{
	return StasisAbility && StasisAbility->IsStasisActive();
}
