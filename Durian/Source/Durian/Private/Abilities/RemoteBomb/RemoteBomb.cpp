#include "Abilities/RemoteBomb/RemoteBomb.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Framework/Utility/NiagaraEffectUtility.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "NiagaraSystem.h"

ARemoteBomb::ARemoteBomb()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetCanBeDamaged(false);
}

void ARemoteBomb::BeginPlay()
{
	Super::BeginPlay();

	UPrimitiveComponent* CollisionRoot = Cast<UPrimitiveComponent>(GetRootComponent());
	if (!IsValid() || !CollisionRoot)
	{
		return;
	}
	CollisionRoot->SetCollisionProfileName(TEXT("PhysicsActor"));
	CollisionRoot->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	TArray<UStaticMeshComponent*> VisualMeshes;
	GetComponents<UStaticMeshComponent>(VisualMeshes);
	for (UStaticMeshComponent* VisualMesh : VisualMeshes)
	{
		if (VisualMesh && VisualMesh != CollisionRoot)
		{
			VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			VisualMesh->SetSimulatePhysics(false);
		}
	}

}

bool ARemoteBomb::IsValid() const
{
	if (!Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		UE_LOG(LogTemp, Error, TEXT("ARemoteBomb requires a primitive collision component as its Blueprint root."));

		return false;
	}

	return true;
}

void ARemoteBomb::Hold(const float Height)
{
	AActor* BombOwner = GetOwner();
	UPrimitiveComponent* CollisionRoot = Cast<UPrimitiveComponent>(GetRootComponent());
	if (!CollisionRoot || !BombOwner)
	{
		return;
	}

	CollisionRoot->SetSimulatePhysics(false);
	CollisionRoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (const AKzPlayerCharacter* PlayerCharacter = Cast<AKzPlayerCharacter>(BombOwner);
		PlayerCharacter && PlayerCharacter->GetMesh() && PlayerCharacter->GetMesh()->DoesSocketExist(HeldSocketName))
	{
		AttachToComponent(PlayerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeldSocketName);
		SetActorRelativeLocation(HeldSocketOffset);
		SetActorRelativeRotation(HeldSocketRotation);
	}
	else
	{
		// Keep the legacy fallback for non-player owners or bombs whose mesh does
		// not provide a hand socket.
		AttachToActor(BombOwner, FAttachmentTransformRules::KeepWorldTransform);
		SetActorRelativeLocation(FVector(0.0f, 0.0f, Height));
	}
	bHeld = true;
}

void ARemoteBomb::Place(const FVector& Location, const FVector& Impulse)
{
	UPrimitiveComponent* CollisionRoot = Cast<UPrimitiveComponent>(GetRootComponent());
	if (!CollisionRoot)
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	CollisionRoot->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionRoot->SetSimulatePhysics(true);
	bHeld = false;
	if (!Impulse.IsNearlyZero())
	{
		CollisionRoot->AddImpulse(Impulse, NAME_None, true);
	}
}

void ARemoteBomb::PlayExplosionEffect(const float Radius) const
{
	if (ExplosionEffect)
	{
		FNiagaraEffectUtility::SpawnAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
		return;
	}

	const FSoftObjectPath DefaultExplosionEffect(TEXT("/Game/Resources/VFX/RemoteBomb/Niagara/NS_RemoteBombExplosion.NS_RemoteBombExplosion"));

	if (FNiagaraEffectUtility::SpawnAtLocation(GetWorld(), DefaultExplosionEffect, GetActorLocation()))
	{
		return;
	}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 24, FColor::Orange, false, 0.75f, 0, 2.0f);
#endif
}
