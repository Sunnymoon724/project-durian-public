#include "Abilities/RemoteBomb/RemoteBomb.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

ARemoteBomb::ARemoteBomb()
{
	PrimaryActorTick.bCanEverTick = false;
	BombMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BombMesh"));
	RootComponent = BombMesh;
	BombMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	BombMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BombMesh->SetSimulatePhysics(false);
	BombMesh->SetEnableGravity(true);
	SetCanBeDamaged(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		BombMesh->SetStaticMesh(SphereMesh.Object);
	}
	BombMesh->SetWorldScale3D(FVector(0.5f));
}

void ARemoteBomb::Initialize(const ERemoteBombShape InShape, AKzPlayerCharacter* InOwner)
{
	Shape = InShape;
	OwningCharacter = InOwner;
	if (Shape == ERemoteBombShape::Cube)
	{
		UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (CubeMesh)
		{
			BombMesh->SetStaticMesh(CubeMesh);
		}
		BombMesh->SetAngularDamping(8.0f);
		BombMesh->SetLinearDamping(1.0f);
	}
	else
	{
		BombMesh->SetAngularDamping(0.1f);
		BombMesh->SetLinearDamping(0.05f);
	}
}

void ARemoteBomb::Hold(const float Height)
{
	AKzPlayerCharacter* OwnerCharacter = OwningCharacter.Get();
	if (!OwnerCharacter)
	{
		return;
	}
	BombMesh->SetSimulatePhysics(false);
	BombMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachToActor(OwnerCharacter, FAttachmentTransformRules::KeepWorldTransform);
	SetActorRelativeLocation(FVector(0.0f, 0.0f, Height));
	bHeld = true;
}

void ARemoteBomb::Place(const FVector& Location, const FVector& Impulse)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	BombMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BombMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BombMesh->SetSimulatePhysics(true);
	bHeld = false;
	if (!Impulse.IsNearlyZero())
	{
		BombMesh->AddImpulse(Impulse, NAME_None, true);
	}
}

void ARemoteBomb::PlayExplosionEffect(const float Radius) const
{
	if (ExplosionEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		return;
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 24, FColor::Orange, false, 0.75f, 0, 2.0f);
#endif
}
