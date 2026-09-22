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
	PrimaryActorTick.bCanEverTick = false;

	IceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IceMesh"));
	RootComponent = IceMesh;
	IceMesh->SetCollisionProfileName(TEXT("BlockAll"));
	IceMesh->SetSimulatePhysics(false);
	IceMesh->SetMobility(EComponentMobility::Movable);
	IceMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 2.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		IceMesh->SetStaticMesh(CubeMesh.Object);
	}

	ReactionComponent = CreateDefaultSubobject<UAbilityReactionComponent>(TEXT("AbilityReaction"));
	ReactionComponent->SetReactionType(EAbilityReactionType::CryonicTarget);
	Tags.AddUnique(TEXT("IcePillar"));
}

void AIcePillar::PlayDestroyEffect()
{
	if (!GetWorld())
	{
		return;
	}

	UNiagaraSystem* DissolveSystem = Cast<UNiagaraSystem>(StaticLoadObject(
		UNiagaraSystem::StaticClass(), nullptr,
		TEXT("/Game/Resources/VFX/Ice/Niagara/NS_IcePillarDissolve.NS_IcePillarDissolve")));
	if (DissolveSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), DissolveSystem, GetActorLocation(), GetActorRotation(), GetActorScale3D(), true, true);
	}
}
