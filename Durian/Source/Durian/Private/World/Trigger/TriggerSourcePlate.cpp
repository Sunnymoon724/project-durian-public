#include "World/Trigger/TriggerSourcePlate.h"

#include "Abilities/Components/AbilityReactionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATriggerSourcePlate::ATriggerSourcePlate()
{
	PrimaryActorTick.bCanEverTick = false;
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

	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	BoxCollision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	BoxCollision->SetGenerateOverlapEvents(true);
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ATriggerSourcePlate::HandleBoxBeginOverlap);
	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ATriggerSourcePlate::HandleBoxEndOverlap);

	if (IsValid(RequiredBlock))
	{
		TArray<UPrimitiveComponent*> BlockComponents;
		RequiredBlock->GetComponents<UPrimitiveComponent>(BlockComponents);
		for (UPrimitiveComponent* BlockComponent : BlockComponents)
		{
			if (BlockComponent)
			{
				BlockComponent->SetGenerateOverlapEvents(true);
			}
		}

		if (UAbilityReactionComponent* Reaction = RequiredBlock->FindComponentByClass<UAbilityReactionComponent>())
		{
			MagnesisHeldChangedHandle = Reaction->OnMagnesisHeldStateChanged.AddUObject(this, &ATriggerSourcePlate::HandleRequiredBlockHeldChanged);
		}
	}

	// Let the Blueprint target components subscribe before publishing an initially pressed state.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ATriggerSourcePlate::Initialize);
	}
}

void ATriggerSourcePlate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(RequiredBlock))
	{
		if (UAbilityReactionComponent* Reaction = RequiredBlock->FindComponentByClass<UAbilityReactionComponent>())
		{
			Reaction->OnMagnesisHeldStateChanged.Remove(MagnesisHeldChangedHandle);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ATriggerSourcePlate::Initialize()
{
	bInitialized = true;
	bPlateEnabled = StartMode != ETriggerPlateStartMode::Off;

	if (!bPlateEnabled || !IsRequiredBlockOverlapping())
	{
		return;
	}

	const UAbilityReactionComponent* Reaction = RequiredBlock->FindComponentByClass<UAbilityReactionComponent>();
	if (!Reaction || !Reaction->IsMagnesisHeld())
	{
		BroadcastTriggerState(true);
	}
}

void ATriggerSourcePlate::HandleBoxBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!bInitialized || !bPlateEnabled || OtherActor != RequiredBlock)
	{
		return;
	}

	const UAbilityReactionComponent* Reaction = RequiredBlock->FindComponentByClass<UAbilityReactionComponent>();
	if (!Reaction || !Reaction->IsMagnesisHeld())
	{
		BroadcastTriggerState(true);
	}
}

void ATriggerSourcePlate::HandleBoxEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	if (bInitialized && bPlateEnabled && OtherActor == RequiredBlock && !IsRequiredBlockOverlapping())
	{
		BroadcastTriggerState(false);
	}
}

void ATriggerSourcePlate::HandleRequiredBlockHeldChanged(const bool bIsHeld)
{
	if (bInitialized && bPlateEnabled && IsRequiredBlockOverlapping())
	{
		BroadcastTriggerState(!bIsHeld);
	}
}

bool ATriggerSourcePlate::IsRequiredBlockOverlapping() const
{
	return IsValid(RequiredBlock) && BoxCollision && BoxCollision->IsOverlappingActor(RequiredBlock);
}
