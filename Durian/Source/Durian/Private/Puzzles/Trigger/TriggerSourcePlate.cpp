#include "Puzzles/Trigger/TriggerSourcePlate.h"
#include "Abilities/Core/PlayerAbilityComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DataAssets/PuzzleVisualProfiles.h"
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
	BoxCollision = FindComponentByClass<UBoxComponent>();

	if (!BoxCollision)
	{
		UE_LOG(LogTemp, Error, TEXT("ATriggerSourcePlate requires a UBoxComponent."));

		return;
	}

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ATriggerSourcePlate::Initialize);
	}
}

void ATriggerSourcePlate::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdatePlateState();
}

void ATriggerSourcePlate::Initialize()
{
	InitializePlateMaterials();

	Super::Initialize();

	RefreshColor();

	if (const AKzPlayerCharacter* Player = Cast<AKzPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		PlayerAbilityComponent = Player->GetAbilityComponent();
	}

	if (!IsEnable())
	{
		SetActorTickEnabled(false);

		return;
	}

	UpdatePlateState();
}

void ATriggerSourcePlate::SetCurrentState(const ETriggerSourceState NewState)
{
	const ETriggerSourceState PreviousState = GetCurrentState();

	Super::SetCurrentState(NewState);

	if (PreviousState != GetCurrentState())
	{
		RefreshColor();
	}
}

void ATriggerSourcePlate::OnEnabledChanged(const bool bEnabled)
{
	if (!IsInitialized())
	{
		return;
	}

	if (!bEnabled)
	{
		SetActorTickEnabled(false);

		return;
	}

	UpdatePlateState();
}

void ATriggerSourcePlate::HandleBoxBeginOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!IsEnable())
	{
		return;
	}

	UpdatePlateState();
}

void ATriggerSourcePlate::HandleBoxEndOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32)
{
	if (IsEnable())
	{
		UpdatePlateState();
	}
}

void ATriggerSourcePlate::UpdatePlateState()
{
	if (!IsEnable() || !BoxCollision)
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

	if (bHasUnheldPhysicsObjectOverlapping)
	{
		SetCurrentState(ETriggerSourceState::Active);
	}
	else
	{
		SetCurrentState(ETriggerSourceState::Ready);
	}

	SetActorTickEnabled(bHasPhysicsObjectOverlapping);
}

void ATriggerSourcePlate::InitializePlateMaterials()
{
	MaterialInstances.Reset();

	for (const FTriggerSourcePlateMaterialTarget& MaterialTarget : MaterialTargetArray)
	{
		UStaticMeshComponent* VisualMeshComponent = Cast<UStaticMeshComponent>(MaterialTarget.ComponentReference.GetComponent(this));

		if (!IsValid(VisualMeshComponent))
		{
			UE_LOG(LogTemp, Error, TEXT("ATriggerSourcePlate has an invalid Mesh Component in Material Targets."));

			continue;
		}

		const int32 MaterialIndex = VisualMeshComponent->GetMaterialIndex(MaterialTarget.MaterialSlotName);

		if (MaterialIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Error, TEXT("ATriggerSourcePlate has an invalid Material Slot Name: %s."), *MaterialTarget.MaterialSlotName.ToString());

			continue;
		}

		if (UMaterialInstanceDynamic* MaterialInstance = VisualMeshComponent->CreateDynamicMaterialInstance(MaterialIndex))
		{
			MaterialInstances.Add(MaterialInstance);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ATriggerSourcePlate could not create a dynamic material for slot %s."), *MaterialTarget.MaterialSlotName.ToString());
		}

	}
}

void ATriggerSourcePlate::RefreshColor()
{
	const auto& [OffColor, ReadyColor, ActiveColor, EmissivePower] = FPuzzleVisualProfiles::GetTriggerSourcePlate();
	const ETriggerSourceState PlateState = GetCurrentState();

	FLinearColor StateColor;

	switch (PlateState)
	{
		case ETriggerSourceState::Off:
			StateColor = OffColor;
			break;
		case ETriggerSourceState::Active:
			StateColor = ActiveColor;
			break;
		case ETriggerSourceState::Ready:
			StateColor = ReadyColor;
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(PlateState));
			StateColor = OffColor;
			break;
	}

	for (UMaterialInstanceDynamic* MaterialInstance : MaterialInstances)
	{
		if (!IsValid(MaterialInstance))
		{
			continue;
		}

		MaterialInstance->SetVectorParameterValue(TEXT("Emissive_Color"), StateColor);
		MaterialInstance->SetScalarParameterValue(TEXT("Emissive_Power"), EmissivePower);
	}
}
