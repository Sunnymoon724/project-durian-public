#include "Abilities/Cryonis/IcePillar.h"

#include "Abilities/Components/AbilityReactionComponent.h"
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
	IceMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -HalfHeight + 1.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (CubeMesh.Succeeded())
	{
		IceMesh->SetStaticMesh(CubeMesh.Object);
	}

	ReactionComponent = CreateDefaultSubobject<UAbilityReactionComponent>(TEXT("AbilityReaction"));
	ReactionComponent->SetReactionType(EAbilityReactionType::CryonicTarget);
	Tags.AddUnique(TEXT("IcePillar"));
}

void AIcePillar::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SpawnAnimationElapsed = FMath::Min(SpawnAnimationElapsed + DeltaSeconds, SpawnAnimationDuration);
	const float Progress = FMath::Clamp(
		SpawnAnimationElapsed / FMath::Max(0.01f, SpawnAnimationDuration),
		0.0f,
		1.0f);
	const float EasedProgress = FMath::InterpEaseOut(0.0f, 1.0f, Progress, 2.0f);

	IceMesh->SetRelativeScale3D(FVector(
		HorizontalScale,
		HorizontalScale,
		FMath::Max(0.02f, FullHeightScale * EasedProgress)));
	IceMesh->SetRelativeLocation(FVector(0.0f, 0.0f, (-HalfHeight + 1.0f) * (1.0f - EasedProgress)));

	if (Progress >= 1.0f)
	{
		SetActorTickEnabled(false);
	}
}

void AIcePillar::PlayDestroyEffect()
{
	if (!GetWorld())
	{
		return;
	}

	UNiagaraSystem* DissolveSystem = Cast<UNiagaraSystem>(StaticLoadObject(
		UNiagaraSystem::StaticClass(),
		nullptr,
		TEXT("/Game/Resources/VFX/Ice/Niagara/NS_IcePillarDissolve.NS_IcePillarDissolve")));

	if (DissolveSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DissolveSystem,
			GetActorLocation(),
			GetActorRotation(),
			GetActorScale3D(),
			true,
			true);
	}
}
