#include "Abilities/Core/PlayerAbilityComponent.h"
#include "Abilities/Cryonis/CryonisAbility.h"
#include "Abilities/Magnesis/MagnesisAbility.h"
#include "Abilities/RemoteBomb/RemoteBombAbility.h"
#include "Abilities/Stasis/StasisAbility.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"

UPlayerAbilityComponent::UPlayerAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

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
		CurrentAbility->HandleInput(EAbilityInput::Cancel);
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
	else if (PreviousAbility == StasisAbility.Get() && PreviousAbility != CurrentAbility)
	{
		StasisAbility->HandleAbilityDeselected();
	}
	else if (PreviousAbility && PreviousAbility != CurrentAbility)
	{
		PreviousAbility->HandleInput(EAbilityInput::Cancel);
	}

	Character->OnAbilityChanged.Broadcast(NewAbility);
}

void UPlayerAbilityComponent::HandleInput(const EAbilityInput Input, const float AxisValue) const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleInput(Input, AxisValue);
	}
}

float UPlayerAbilityComponent::GetRemoteBombCooldown(const ERemoteBombShape Shape) const
{
	return RemoteBombAbility ? RemoteBombAbility->GetCooldownRemaining(Shape) : 0.0f;
}

float UPlayerAbilityComponent::GetRemoteBombSphereCooldown() const
{
	return GetRemoteBombCooldown(ERemoteBombShape::Sphere);
}

float UPlayerAbilityComponent::GetRemoteBombCubeCooldown() const
{
	return GetRemoteBombCooldown(ERemoteBombShape::Cube);
}

bool UPlayerAbilityComponent::HasRemoteBomb(const ERemoteBombShape Shape) const
{
	return RemoteBombAbility && RemoteBombAbility->HasBomb(Shape);
}

bool UPlayerAbilityComponent::HasRemoteBombSphere() const
{
	return HasRemoteBomb(ERemoteBombShape::Sphere);
}

bool UPlayerAbilityComponent::HasRemoteBombCube() const
{
	return HasRemoteBomb(ERemoteBombShape::Cube);
}

bool UPlayerAbilityComponent::IsRemoteBombInstalled(const ERemoteBombShape Shape) const
{
	return RemoteBombAbility && RemoteBombAbility->IsBombInstalled(Shape);
}

bool UPlayerAbilityComponent::IsRemoteBombSphereInstalled() const
{
	return IsRemoteBombInstalled(ERemoteBombShape::Sphere);
}

bool UPlayerAbilityComponent::IsRemoteBombCubeInstalled() const
{
	return IsRemoteBombInstalled(ERemoteBombShape::Cube);
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
