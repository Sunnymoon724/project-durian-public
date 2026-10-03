#include "Abilities/Cryonis/IcePlacementPreview.h"
#include "DataAssets/GameConstantsDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

AIcePlacementPreview::AIcePlacementPreview()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AIcePlacementPreview::BeginPlay()
{
	Super::BeginPlay();

	SetActorHiddenInGame(true);

	PreviewPlane = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("Plane")));
	PreviewPillar = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("Pillar")));

	if (!IsSetupValid())
	{
		UE_LOG(LogTemp, Error, TEXT("AIcePlacementPreview requires StaticMeshComponents named Plane and Pillar in BP_IcePlacementPreview."));

		return;
	}

	if (UMaterialInterface* Material = PreviewPlane->GetMaterial(0))
	{
		PreviewMaterial = UMaterialInstanceDynamic::Create(Material, this);
		PreviewPlane->SetMaterial(0, PreviewMaterial);
	}

	PillarHeight = UGameConstantsDataAsset::Get()->IcePillarHeight;
	PillarHorizontalScale = UGameConstantsDataAsset::Get()->IcePillarHorizontalScale;
	PillarAnimationDuration = UGameConstantsDataAsset::Get()->IcePillarPreviewAnimationDuration;

	PreviewPlane->SetRelativeScale3D(FVector(PillarHorizontalScale, PillarHorizontalScale, 1.0f));
}

bool AIcePlacementPreview::IsSetupValid() const
{
	return PreviewPlane && PreviewPillar;
}

void AIcePlacementPreview::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsSetupValid())
	{
		return;
	}

	const bool bHasAnimatedPreview = PreviewPillar->IsVisible();

	if (!bHasAnimatedPreview)
	{
		return;
	}

	const float Duration = FMath::Max(0.01f, PillarAnimationDuration);

	PillarPreviewElapsed = FMath::Fmod(PillarPreviewElapsed + DeltaSeconds, Duration);

	const float Progress = PillarPreviewElapsed / Duration;
	const float EasedProgress = FMath::InterpEaseOut(0.0f, 1.0f, Progress, 2.0f);

	const float MeshHeight = PreviewPillar->GetStaticMesh() ? PreviewPillar->GetStaticMesh()->GetBoundingBox().GetSize().Z : 100.0f;
	const float PillarHalfHeight = PillarHeight*0.5f;

	PreviewPillar->SetRelativeScale3D(FVector(PillarHorizontalScale, PillarHorizontalScale, FMath::Max(0.02f, PillarHeight / FMath::Max(1.0f, MeshHeight) * EasedProgress)));
	PreviewPillar->SetRelativeLocation(FVector(0.0f, 0.0f, -PillarHalfHeight));
}

void AIcePlacementPreview::SetPreviewState(const FVector& Location, const bool bVisible, const bool bCanSpawn)
{
	if (!IsSetupValid())
	{
		return;
	}

	const float PillarHalfHeight = PillarHeight*0.5f;

	PreviewPlane->SetRelativeLocation(FVector(0.0f, 0.0f, -PillarHalfHeight + 2.0f));

	SetActorLocation(Location);
	SetActorHiddenInGame(!bVisible);

	PreviewPlane->SetVisibility(bVisible, true);
	PreviewPillar->SetVisibility(bVisible && bCanSpawn, true);

	if (bVisible && bCanSpawn && (!bHasPreviewLocation || !LastPreviewLocation.Equals(Location, 0.5f)))
	{
		LastPreviewLocation = Location;
		bHasPreviewLocation = true;
		PillarPreviewElapsed = 0.0f;

		PreviewPillar->SetRelativeScale3D(FVector(PillarHorizontalScale, PillarHorizontalScale, 0.02f));
		PreviewPillar->SetRelativeLocation(FVector(0.0f, 0.0f, -PillarHalfHeight));
	}

	const auto UpdatePreviewMaterial = [bCanSpawn](UMaterialInstanceDynamic* Material)
	{
		if (!Material)
		{
			return;
		}

		Material->SetScalarParameterValue(TEXT("IsValid"), bCanSpawn ? 1.0f : 0.0f);
	};

	UpdatePreviewMaterial(PreviewMaterial);

	SetActorTickEnabled(bVisible && bCanSpawn);
}

void AIcePlacementPreview::ClearPreview()
{
	if (!IsSetupValid())
	{
		SetActorTickEnabled(false);
		SetActorHiddenInGame(true);
	}
	else
	{
		PreviewPillar->SetVisibility(false, true);

		SetActorTickEnabled(false);

		PreviewPlane->SetVisibility(false, true);

		SetActorHiddenInGame(true);

		bHasPreviewLocation = false;
	}
}
