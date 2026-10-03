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

	MagnesisAbility = NewObject<UMagnesisAbility>(this);
	MagnesisAbility->Initialize(Character);

	StasisAbility = NewObject<UStasisAbility>(this);
	StasisAbility->Initialize(Character);

	RemoteBombAbility = NewObject<URemoteBombAbility>(this);
	RemoteBombAbility->Initialize(Character);
	RemoteBombAbility->SetBombClasses(RemoteBombSphereClass, RemoteBombCubeClass);

	CryonisAbility = NewObject<UCryonisAbility>(this);
	CryonisAbility->Initialize(Character);

	CurrentAbility = MagnesisAbility;
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

	SetMagnesisHeldActor(nullptr);

	Super::EndPlay(EndPlayReason);
}

void UPlayerAbilityComponent::SetMagnesisHeldActor(AActor* Actor)
{
	if (MagnesisHeldActor.Get() == Actor)
	{
		return;
	}

	MagnesisHeldActor = Actor;
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

	UAbility* PreviousAbility = CurrentAbility.Get();
	const EAbilityType PreviousAbilityType = Character->GetCurrentAbilityType();

	if (PreviousAbility && PreviousAbilityType != NewAbility)
	{
		PreviousAbility->OnDeselected();
	}

	Character->CurrentAbilityType = NewAbility;

	CurrentAbility = GetAbility(NewAbility);

	Character->OnAbilityChanged.Broadcast(NewAbility);
}

void UPlayerAbilityComponent::HandleInput(const EAbilityInput Input, const float AxisValue) const
{
	if (CurrentAbility)
	{
		CurrentAbility->HandleInput(Input, AxisValue);
	}
}

void UPlayerAbilityComponent::HandleAttackHit(const FHitResult& Hit, const FVector& AttackDirection) const
{
	if (StasisAbility)
	{
		StasisAbility->HandleAttackHit(Hit, AttackDirection);
	}
}

UAbility* UPlayerAbilityComponent::GetAbility(const EAbilityType AbilityType) const
{
	switch (AbilityType)
	{
	case EAbilityType::Magnesis:
		return MagnesisAbility.Get();
	case EAbilityType::Cryonis:
		return CryonisAbility.Get();
	case EAbilityType::Stasis:
		return StasisAbility.Get();
	case EAbilityType::RemoteBombSphere:
	case EAbilityType::RemoteBombCube:
		return RemoteBombAbility.Get();
	default:
		UE_LOG(LogTemp, Error, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(AbilityType));
		return nullptr;
	}
}
