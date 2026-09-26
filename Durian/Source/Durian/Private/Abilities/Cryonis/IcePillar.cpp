#include "Abilities/Cryonis/IcePillar.h"
#include "Abilities/Components/AbilityReactionComponent.h"
#include "Constants/GameConstantsDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

AIcePillar::AIcePillar()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	IceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IceMesh"));
	IceMesh->SetupAttachment(SceneRoot);
	IceMesh->SetCollisionProfileName(TEXT("BlockAll"));
	IceMesh->SetSimulatePhysics(false);
	IceMesh->SetMobility(EComponentMobility::Movable);
	IceMesh->SetRelativeScale3D(FVector(HorizontalScale, HorizontalScale, 0.02f));
	IceMesh->SetRelativeLocation(FVector::ZeroVector);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeMesh.Succeeded())
	{
		IceMesh->SetStaticMesh(CubeMesh.Object);
	}

	ReactionComponent = CreateDefaultSubobject<UAbilityReactionComponent>(TEXT("AbilityReaction"));
	ReactionComponent->SetReactionType(EAbilityReactionType::CryonicTarget);

	Tags.AddUnique(TEXT("IcePillar"));
}

void AIcePillar::BeginPlay()
{
	Super::BeginPlay();
	IceMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -UGameConstantsDataAsset::Get()->IcePillarHeight * 0.5f));
}

void AIcePillar::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float SpawnDuration = UGameConstantsDataAsset::Get()->IcePillarSpawnAnimationDuration;
	SpawnAnimationElapsed = FMath::Min(SpawnAnimationElapsed + DeltaSeconds, SpawnDuration);

	const float Progress = FMath::Clamp(SpawnAnimationElapsed / FMath::Max(0.01f, SpawnDuration),0.0f,1.0f);
	const float EasedProgress = FMath::InterpEaseOut(0.0f, 1.0f, Progress, 2.0f);

	const float MeshHeight = IceMesh->GetStaticMesh() ? IceMesh->GetStaticMesh()->GetBoundingBox().GetSize().Z : 100.0f;
	const float HeightScale = UGameConstantsDataAsset::Get()->IcePillarHeight / FMath::Max(1.0f, MeshHeight);
	IceMesh->SetRelativeScale3D(FVector(HorizontalScale, HorizontalScale, FMath::Max(0.02f, HeightScale * EasedProgress)));

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

	const TCHAR* EffectPath = bShatter ? TEXT("/Game/Resources/VFX/Cryonis/Niagara/NS_IcePillarShatter.NS_IcePillarShatter") : TEXT("/Game/Resources/VFX/Cryonis/Niagara/NS_IcePillarDissolve.NS_IcePillarDissolve");
	UNiagaraSystem* DissolveSystem = Cast<UNiagaraSystem>(StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr, EffectPath));

	if (DissolveSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),DissolveSystem,GetActorLocation(),GetActorRotation(),GetActorScale3D(),true,true);
	}
}
