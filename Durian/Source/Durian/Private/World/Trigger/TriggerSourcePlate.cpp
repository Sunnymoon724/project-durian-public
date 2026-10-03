#include "World/Trigger/TriggerSourcePlate.h"
#include "Abilities/Core/PlayerAbilityComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ATriggerSourcePlate::ATriggerSourcePlate()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ATriggerSourcePlate::BeginPlay()
{
	Super::BeginPlay();

	// The plate volume is authored in the Blueprint so it can be sized and positioned there.
	BoxCollision = Cast<UBoxComponent>(GetDefaultSubobjectByName(TEXT("Box")));

	if (!BoxCollision)
	{
		UE_LOG(LogTemp, Error, TEXT("ATriggerSourcePlate requires a UBoxComponent named Box."));
		return;
	}

	// Let the Blueprint target components subscribe before publishing an initially pressed state.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ATriggerSourcePlate::Initialize);
	}
}

void ATriggerSourcePlate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATriggerSourcePlate::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdatePlateState();
}

void ATriggerSourcePlate::Initialize()
{
	if (AKzPlayerCharacter* Player = Cast<AKzPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		PlayerAbilityComponent = Player->GetAbilityComponent();
	}

	bInitialized = true;
	bPlateEnabled = StartMode != ETriggerPlateStartMode::Off;

	if (!bPlateEnabled)
	{
		SetActorTickEnabled(false);
		return;
	}

	UpdatePlateState();
}

void ATriggerSourcePlate::HandleBoxBeginOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!bInitialized || !bPlateEnabled)
	{
		return;
	}

	UpdatePlateState();
}

void ATriggerSourcePlate::HandleBoxEndOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32)
{
	if (bInitialized && bPlateEnabled)
	{
		UpdatePlateState();
	}
}

void ATriggerSourcePlate::UpdatePlateState()
{
	if (!bInitialized || !bPlateEnabled)
	{
		SetActorTickEnabled(false);
		return;
	}

	const UPlayerAbilityComponent* AbilityComponent = PlayerAbilityComponent.Get();
	const AActor* HeldActor = AbilityComponent ? AbilityComponent->GetMagnesisHeldActor() : nullptr;
	TArray<UPrimitiveComponent*> OverlappingComponents;
	BoxCollision->GetOverlappingComponents(OverlappingComponents);
	bool bHasPhysicsObjectOverlapping = false;
	bool bHasUnheldPhysicsObjectOverlapping = false;

	for (const UPrimitiveComponent* Component : OverlappingComponents)
	{
		if (!IsValid(Component) || !Component->IsSimulatingPhysics())
		{
			continue;
		}

		bHasPhysicsObjectOverlapping = true;
		bHasUnheldPhysicsObjectOverlapping |= Component->GetOwner() != HeldActor;
	}

	BroadcastTriggerState(bHasUnheldPhysicsObjectOverlapping);
	SetActorTickEnabled(bHasPhysicsObjectOverlapping);
}
