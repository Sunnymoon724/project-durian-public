#include "Abilities/Cryonis/IcePlacementPreview.h"

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

	PreviewPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewPlane"));
	RootComponent = PreviewPlane;
	PreviewPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewPlane->SetCastShadow(false);
	PreviewPlane->SetRelativeLocation(FVector(0.0f, 0.0f, 2.0f));
	PreviewPlane->SetRelativeScale3D(FVector(1.5f, 1.5f, 1.0f));

	SpawnRing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnRing"));
	SpawnRing->SetupAttachment(RootComponent);
	SpawnRing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnRing->SetCastShadow(false);
	SpawnRing->SetRelativeLocation(FVector(0.0f, 0.0f, 3.0f));
	SpawnRing->SetRelativeScale3D(FVector(1.0f));
	SpawnRing->SetVisibility(false, true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		PreviewPlane->SetStaticMesh(PlaneMesh.Object);
		SpawnRing->SetStaticMesh(PlaneMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PreviewMaterialAsset(TEXT("/Game/Resources/VFX/Ice/Materials/MI_IceSpawnPreview.MI_IceSpawnPreview"));
	if (PreviewMaterialAsset.Succeeded())
	{
		PreviewMaterial = UMaterialInstanceDynamic::Create(PreviewMaterialAsset.Object, this);
		PreviewPlane->SetMaterial(0, PreviewMaterial);
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

	if (!RingMaterial || !SpawnRing->IsVisible())
	{
		return;
	}

	SpawnRingElapsed += DeltaSeconds;
	const float Progress = FMath::Clamp(SpawnRingElapsed / SpawnRingDuration, 0.0f, 1.0f);
	const float OuterRadius = FMath::Lerp(0.16f, 0.48f, Progress);
	const float RingWidth = RingMaterial->K2_GetScalarParameterValue(TEXT("RingWidth"));
	RingMaterial->SetScalarParameterValue(TEXT("OuterRadius"), OuterRadius);
	RingMaterial->SetScalarParameterValue(TEXT("InnerRadius"), FMath::Max(0.0f, OuterRadius - RingWidth));
	RingMaterial->SetScalarParameterValue(TEXT("RingOpacity"), FMath::Square(1.0f - Progress));
}

void AIcePlacementPreview::SetPreviewState(const FVector& Location, const bool bVisible, const bool bCanSpawn)
{
	SetActorLocation(Location);
	SetActorHiddenInGame(!bVisible);
	PreviewPlane->SetVisibility(bVisible, true);

	if (PreviewMaterial)
	{
		PreviewMaterial->SetVectorParameterValue(TEXT("PreviewColor"), bCanSpawn
			? FLinearColor(0.05f, 0.75f, 1.0f)
			: FLinearColor(1.0f, 0.10f, 0.35f));
		PreviewMaterial->SetScalarParameterValue(TEXT("EmissiveStrength"), bCanSpawn ? 3.0f : 0.8f);
		PreviewMaterial->SetScalarParameterValue(TEXT("Opacity"), bCanSpawn ? 0.45f : 0.25f);
		PreviewMaterial->SetScalarParameterValue(TEXT("PulseSpeed"), bCanSpawn ? 1.2f : 0.0f);
	}
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

	UNiagaraSystem* SplashSystem = Cast<UNiagaraSystem>(StaticLoadObject(
		UNiagaraSystem::StaticClass(), nullptr,
		TEXT("/Game/Resources/VFX/Ice/Niagara/NS_IceSpawnSplash.NS_IceSpawnSplash")));
	if (SplashSystem && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SplashSystem, GetActorLocation(), FRotator::ZeroRotator, FVector::OneVector, true, true);
	}
}

void AIcePlacementPreview::HideSpawnRing()
{
	SpawnRing->SetVisibility(false, true);
	SetActorTickEnabled(false);
}

void AIcePlacementPreview::ClearPreview()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnRingTimer);
	}
	SpawnRing->SetVisibility(false, true);
	SetActorTickEnabled(false);
	PreviewPlane->SetVisibility(false, true);
	SetActorHiddenInGame(true);
}
