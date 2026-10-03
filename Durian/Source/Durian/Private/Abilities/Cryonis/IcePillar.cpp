#include "Abilities/Cryonis/IcePillar.h"
#include "DataAssets/GameConstantsDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Utilities/NiagaraEffectUtility.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

AIcePillar::AIcePillar()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AIcePillar::BeginPlay()
{
	Super::BeginPlay();

	IcePillar = Cast<UStaticMeshComponent>(GetDefaultSubobjectByName(TEXT("Pillar")));

	if (!IcePillar)
	{
		UE_LOG(LogTemp, Error, TEXT("AIcePillar requires StaticMeshComponents named Pillar in BP_IcePillar."));

		return;
	}

	if (ShatterEffect.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("AIcePillar: ShatterEffect is not assigned in BP_IcePillar."));

		return;
	}

	if (!ShatterEffect.LoadSynchronous())
	{
		UE_LOG(LogTemp, Error, TEXT("AIcePillar: ShatterEffect asset could not be loaded: %s"), *ShatterEffect.ToString());

		return;
	}

	if (DissolveEffect.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("AIcePillar: DissolveEffect is not assigned in BP_IcePillar."));

		return;
	}

	if (!DissolveEffect.LoadSynchronous())
	{
		UE_LOG(LogTemp, Error, TEXT("AIcePillar: DissolveEffect asset could not be loaded: %s"), *DissolveEffect.ToString());

		return;
	}

	PillarHeight = UGameConstantsDataAsset::Get()->IcePillarHeight;
	PillarAnimationDuration = UGameConstantsDataAsset::Get()->IcePillarSpawnAnimationDuration;
	PillarHorizontalScale = UGameConstantsDataAsset::Get()->IcePillarHorizontalScale;

	IcePillar->SetRelativeScale3D(FVector(PillarHorizontalScale, PillarHorizontalScale, 0.02f));
	IcePillar->SetRelativeLocation(FVector(0.0f, 0.0f, -PillarHeight * 0.5f));
}

bool AIcePillar::IsSetupValid() const
{
	return IcePillar && ShatterEffect && DissolveEffect;
}

void AIcePillar::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsSetupValid())
	{
		SetActorTickEnabled(false);

		return;
	}

	const float AnimationDuration = FMath::Max(0.01f, PillarAnimationDuration);
	SpawnAnimationElapsed = FMath::Min(SpawnAnimationElapsed + DeltaSeconds, AnimationDuration);

	const float Progress = SpawnAnimationElapsed / AnimationDuration;
	const float EasedProgress = FMath::InterpEaseOut(0.0f, 1.0f, Progress, 2.0f);

	const UStaticMesh* StaticMesh = IcePillar->GetStaticMesh();
	const float MeshHeight = StaticMesh ? StaticMesh->GetBoundingBox().GetSize().Z : 100.0f;
	const float HeightScale = PillarHeight / FMath::Max(1.0f, MeshHeight);
	const float CurrentHeightScale = FMath::Max(0.02f, HeightScale * EasedProgress);

	IcePillar->SetRelativeScale3D(FVector(PillarHorizontalScale, PillarHorizontalScale, CurrentHeightScale));

	if (Progress >= 1.0f)
	{
		SetActorTickEnabled(false);
	}
}

void AIcePillar::PlayDestroyEffect(const bool bShatter) const
{
	if (!GetWorld())
	{
		return;
	}

	if (bShatter)
	{
		const FSoftObjectPath FractureEffect(TEXT("/Game/Resources/VFX/Cryonis/Niagara/NS_IcePillarFracture.NS_IcePillarFracture"));
		if (FNiagaraEffectUtility::SpawnAtLocation(GetWorld(), FractureEffect, GetActorLocation(), GetActorRotation(), GetActorScale3D()))
		{
			return;
		}
	}

	const TSoftObjectPtr<UNiagaraSystem>& Effect = bShatter ? ShatterEffect : DissolveEffect;
	FNiagaraEffectUtility::SpawnAtLocation(GetWorld(), Effect, GetActorLocation(), GetActorRotation(), GetActorScale3D());
}
