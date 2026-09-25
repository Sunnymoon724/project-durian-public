#include "Abilities/Cryonis/IcePlacementPreview.h"
#include "Abilities/Cryonis/IcePillar.h"
#include "Constants/GameConstantsDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

AIcePlacementPreview::AIcePlacementPreview()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	PreviewPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewPlane"));
	PreviewPlane->SetupAttachment(SceneRoot);
	PreviewPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewPlane->SetCastShadow(false);
	PreviewPlane->SetRelativeLocation(FVector::ZeroVector);
	PreviewPlane->SetRelativeScale3D(FVector(AIcePillar::HorizontalScale, AIcePillar::HorizontalScale, 1.0f));

	PreviewPillar = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewPillar"));
	PreviewPillar->SetupAttachment(SceneRoot);
	PreviewPillar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewPillar->SetCastShadow(false);
	PreviewPillar->SetVisibility(false, true);

	SpawnRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnRing"));
	SpawnRing->SetupAttachment(SceneRoot);
	SpawnRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnRing->SetCastShadow(false);
	SpawnRing->SetRelativeLocation(FVector::ZeroVector);
	SpawnRing->SetRelativeScale3D(FVector(1.0f));
	SpawnRing->SetVisibility(false, true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));

	if (PlaneMesh.Succeeded())
	{
		PreviewPlane->SetStaticMesh(PlaneMesh.Object);
		SpawnRing->SetStaticMesh(PlaneMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeMesh.Succeeded())
	{
		PreviewPillar->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PreviewMaterialAsset(TEXT("/Game/Resources/VFX/Cryonis/Materials/MI_IceSpawnPreview.MI_IceSpawnPreview"));

	if (PreviewMaterialAsset.Succeeded())
	{
		PreviewMaterial = UMaterialInstanceDynamic::Create(PreviewMaterialAsset.Object, this);
		PreviewPlane->SetMaterial(0, PreviewMaterial);
		PillarPreviewMaterial = UMaterialInstanceDynamic::Create(PreviewMaterialAsset.Object, this);
		PreviewPillar->SetMaterial(0, PillarPreviewMaterial);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> RingMaterialAsset(TEXT("/Game/Resources/VFX/Cryonis/Materials/MI_IceSpawnRing.MI_IceSpawnRing"));

	if (RingMaterialAsset.Succeeded())
	{
		RingMaterial = UMaterialInstanceDynamic::Create(RingMaterialAsset.Object, this);
		SpawnRing->SetMaterial(0, RingMaterial);
	}

	SetActorHiddenInGame(true);
}

void AIcePlacementPreview::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bHasAnimatedPreview = PreviewPillar->IsVisible() || SpawnRing->IsVisible();

	if (!bHasAnimatedPreview)
	{
		return;
	}

	if (PreviewPillar->IsVisible())
	{
		const float Duration = FMath::Max(0.01f, UGameConstantsDataAsset::Get()->IcePillarPreviewAnimationDuration);
		PillarPreviewElapsed = FMath::Fmod(PillarPreviewElapsed + DeltaSeconds, Duration);
		const float Progress = PillarPreviewElapsed / Duration;
		const float EasedProgress = FMath::InterpEaseOut(0.0f, 1.0f, Progress, 2.0f);

		const float HalfHeight = UGameConstantsDataAsset::Get()->IcePillarHeight * 0.5f;
		const float MeshHeight = PreviewPillar->GetStaticMesh() ? PreviewPillar->GetStaticMesh()->GetBoundingBox().GetSize().Z : 100.0f;
		PreviewPillar->SetRelativeScale3D(FVector(AIcePillar::HorizontalScale, AIcePillar::HorizontalScale, FMath::Max(0.02f, UGameConstantsDataAsset::Get()->IcePillarHeight / FMath::Max(1.0f, MeshHeight) * EasedProgress)));
		PreviewPillar->SetRelativeLocation(FVector(0.0f, 0.0f, -HalfHeight * (1.0f - EasedProgress)));
	}

	if (RingMaterial && SpawnRing->IsVisible())
	{
		SpawnRingElapsed += DeltaSeconds;
		const float Progress = FMath::Clamp(SpawnRingElapsed / SpawnRingDuration, 0.0f, 1.0f);
		const float OuterRadius = FMath::Lerp(0.16f, 0.48f, Progress);
		const float RingWidth = RingMaterial->K2_GetScalarParameterValue(TEXT("RingWidth"));
		RingMaterial->SetScalarParameterValue(TEXT("OuterRadius"), OuterRadius);
		RingMaterial->SetScalarParameterValue(TEXT("InnerRadius"), FMath::Max(0.0f, OuterRadius - RingWidth));
		RingMaterial->SetScalarParameterValue(TEXT("RingOpacity"), FMath::Square(1.0f - Progress));
	}
}

void AIcePlacementPreview::SetPreviewState(const FVector& Location, const bool bVisible, const bool bCanSpawn)
{
	const float HalfHeight = UGameConstantsDataAsset::Get()->IcePillarHeight * 0.5f;
	PreviewPlane->SetRelativeLocation(FVector(0.0f, 0.0f, -HalfHeight + 2.0f));
	SpawnRing->SetRelativeLocation(FVector(0.0f, 0.0f, -HalfHeight + 3.0f));
	SetActorLocation(Location);
	SetActorHiddenInGame(!bVisible);

	PreviewPlane->SetVisibility(bVisible, true);
	PreviewPillar->SetVisibility(bVisible && bCanSpawn, true);

	if (bVisible && bCanSpawn && (!bHasPreviewLocation || !LastPreviewLocation.Equals(Location, 0.5f)))
	{
		LastPreviewLocation = Location;
		bHasPreviewLocation = true;
		PillarPreviewElapsed = 0.0f;
		PreviewPillar->SetRelativeScale3D(FVector(AIcePillar::HorizontalScale, AIcePillar::HorizontalScale, 0.02f));
		PreviewPillar->SetRelativeLocation(FVector(0.0f, 0.0f, -HalfHeight));
	}

	if (PreviewMaterial)
	{
		PreviewMaterial->SetVectorParameterValue(TEXT("PreviewColor"), bCanSpawn ? FLinearColor(0.05f, 0.75f, 1.0f) : FLinearColor(1.0f, 0.10f, 0.35f));
		PreviewMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), bCanSpawn ? 3.0f : 0.8f);
		PreviewMaterial->SetScalarParameterValue(TEXT("Opacity"), bCanSpawn ? 0.45f : 0.25f);
		PreviewMaterial->SetScalarParameterValue(TEXT("PulseSpeed"), bCanSpawn ? 1.2f : 0.0f);
	}

	if (PillarPreviewMaterial)
	{
		PillarPreviewMaterial->SetVectorParameterValue(TEXT("PreviewColor"), bCanSpawn ? FLinearColor(0.05f, 0.75f, 1.0f) : FLinearColor(1.0f, 0.10f, 0.35f));
		PillarPreviewMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), bCanSpawn ? 3.0f : 0.8f);
		PillarPreviewMaterial->SetScalarParameterValue(TEXT("Opacity"), bCanSpawn ? 0.45f : 0.25f);
		PillarPreviewMaterial->SetScalarParameterValue(TEXT("PulseSpeed"), bCanSpawn ? 1.2f : 0.0f);
	}

	SetActorTickEnabled(bVisible && bCanSpawn);
}

void AIcePlacementPreview::PlaySpawnEffect()
{
	SpawnRingElapsed = 0.0f;
	SpawnRingDuration = 0.5f / FMath::Max(0.01f, RingMaterial ? RingMaterial->K2_GetScalarParameterValue(TEXT("RingSpeed")) : 1.0f);

	SpawnRing->SetVisibility(true, true);
	SpawnRing->SetRelativeScale3D(FVector(0.8f));

	SetActorTickEnabled(true);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnRingTimer, this, &AIcePlacementPreview::HideSpawnRing, 0.5f, false);
	}

	UNiagaraSystem* SplashSystem = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr,TEXT("/Game/Resources/VFX/Cryonis/Niagara/NS_IceSpawnSplash.NS_IceSpawnSplash")));

	if (SplashSystem && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SplashSystem, GetActorLocation(), FRotator::ZeroRotator, FVector::OneVector, true, true);
	}
}

void AIcePlacementPreview::HideSpawnRing()
{
	SpawnRing->SetVisibility(false, true);
	PreviewPillar->SetVisibility(false, true);
	SetActorTickEnabled(false);
}

void AIcePlacementPreview::ClearPreview()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnRingTimer);
	}

	SpawnRing->SetVisibility(false, true);
	PreviewPillar->SetVisibility(false, true);

	SetActorTickEnabled(false);

	PreviewPlane->SetVisibility(false, true);

	SetActorHiddenInGame(true);

	bHasPreviewLocation = false;
}
