#include "Abilities/RemoteBomb/RemoteBombAbility.h"

#include "Constants/GameConstantsDataAsset.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

void FRemoteBombAbility::Tick(const float DeltaTime)
{
	for (float& Remaining : CooldownRemaining)
	{
		Remaining = FMath::Max(0.0f, Remaining - DeltaTime);
	}
	if (HeldBomb.IsValid() && (!Character || !Character->GetCharacterMovement() || Character->GetCharacterMovement()->IsFalling()))
	{
		DropHeldBomb();
	}
}

void FRemoteBombAbility::HandleInteract()
{
	if (!Character)
	{
		return;
	}
	if (HeldBomb.IsValid())
	{
		PlaceHeldBomb(false);
		return;
	}

	ERemoteBombShape Shape;
	if (!GetSelectedShape(Shape))
	{
		return;
	}
	ARemoteBomb* Bomb = Bombs[ToIndex(Shape)].Get();
	if (Bomb && !Bomb->IsHeld() && CanPickUp(Bomb))
	{
		Bomb->Hold(UGameConstantsDataAsset::Get()->RemoteBombHoldHeight);
		HeldBomb = Bomb;
	}
}

void FRemoteBombAbility::HandleCancel()
{
	ARemoteBomb* Bomb = HeldBomb.Get();
	if (!Bomb)
	{
		return;
	}
	Bombs[ToIndex(Bomb->GetShape())].Reset();
	HeldBomb.Reset();
	Bomb->Destroy();
}

void FRemoteBombAbility::HandleAbilityUse()
{
	ERemoteBombShape Shape;
	if (!GetSelectedShape(Shape) || !Character || Character->GetCurrentState() != EPlayerState::Normal || HeldBomb.IsValid())
	{
		return;
	}
	if (ARemoteBomb* Bomb = Bombs[ToIndex(Shape)].Get())
	{
		if (!Bomb->IsHeld())
		{
			DetonateBomb(Shape);
		}
		return;
	}
	if (CooldownRemaining[ToIndex(Shape)] <= 0.0f)
	{
		SpawnBomb(Shape);
	}
}

void FRemoteBombAbility::HandleRemoteBombThrow()
{
	ERemoteBombShape Shape;
	if (GetSelectedShape(Shape) && HeldBomb.IsValid() && HeldBomb->GetShape() == Shape)
	{
		PlaceHeldBomb(true);
	}
}

void FRemoteBombAbility::HandleAbilityDeselected()
{
	DropHeldBomb();
}

void FRemoteBombAbility::DropHeldBomb()
{
	PlaceHeldBomb(false);
}

void FRemoteBombAbility::AbortForEndPlay()
{
	HeldBomb.Reset();
	for (int32 Index = 0; Index < 2; ++Index)
	{
		if (ARemoteBomb* Bomb = Bombs[Index].Get())
		{
			Bomb->Destroy();
		}
		Bombs[Index].Reset();
		CooldownRemaining[Index] = 0.0f;
	}
}

float FRemoteBombAbility::GetCooldownRemaining(const ERemoteBombShape Shape) const
{
	return CooldownRemaining[ToIndex(Shape)];
}

bool FRemoteBombAbility::HasBomb(const ERemoteBombShape Shape) const
{
	return Bombs[ToIndex(Shape)].IsValid();
}

bool FRemoteBombAbility::IsBombInstalled(const ERemoteBombShape Shape) const
{
	const ARemoteBomb* Bomb = Bombs[ToIndex(Shape)].Get();
	return Bomb && !Bomb->IsHeld();
}

bool FRemoteBombAbility::GetSelectedShape(ERemoteBombShape& OutShape) const
{
	if (!Character)
	{
		return false;
	}
	if (Character->GetCurrentAbilityType() == EAbilityType::RemoteBombSphere)
	{
		OutShape = ERemoteBombShape::Sphere;
		return true;
	}
	if (Character->GetCurrentAbilityType() == EAbilityType::RemoteBombCube)
	{
		OutShape = ERemoteBombShape::Cube;
		return true;
	}
	return false;
}

void FRemoteBombAbility::SpawnBomb(const ERemoteBombShape Shape)
{
	UWorld* World = Character ? Character->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}
	const UGameConstantsDataAsset* Constants = UGameConstantsDataAsset::Get();
	const FVector HoldLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, Constants->RemoteBombHoldHeight);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RemoteBombSpawn), false, Character);
	QueryParams.AddIgnoredActor(Character);
	if (World->OverlapBlockingTestByChannel(HoldLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Constants->RemoteBombRadius), QueryParams))
	{
		return;
	}
	FActorSpawnParameters Params;
	Params.Owner = Character;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	TSubclassOf<ARemoteBomb> BombClass = Character->GetRemoteBombClass();
	UClass* SpawnClass = BombClass.Get() ? BombClass.Get() : ARemoteBomb::StaticClass();
	ARemoteBomb* Bomb = World->SpawnActor<ARemoteBomb>(SpawnClass, HoldLocation, FRotator::ZeroRotator, Params);
	if (!Bomb)
	{
		return;
	}
	Bomb->Initialize(Shape, Character);
	Bomb->Hold(Constants->RemoteBombHoldHeight);
	Bombs[ToIndex(Shape)] = Bomb;
	HeldBomb = Bomb;
}

void FRemoteBombAbility::DetonateBomb(const ERemoteBombShape Shape)
{
	const int32 Index = ToIndex(Shape);
	ARemoteBomb* Bomb = Bombs[Index].Get();
	if (!Bomb || Bomb->IsHeld())
	{
		return;
	}
	const UGameConstantsDataAsset* Constants = UGameConstantsDataAsset::Get();
	const FVector Origin = Bomb->GetActorLocation();
	const int32 OtherIndex = 1 - Index;
	ARemoteBomb* OtherBomb = Bombs[OtherIndex].Get();
	const bool bChain = OtherBomb && !OtherBomb->IsHeld() && FVector::DistSquared(Origin, OtherBomb->GetActorLocation()) <= FMath::Square(Constants->RemoteBombExplosionRadius);
	Bombs[Index].Reset();
	CooldownRemaining[Index] = Constants->RemoteBombCooldown;
	Bomb->PlayExplosionEffect(Constants->RemoteBombExplosionRadius);
	ApplyExplosion(Bomb, Origin);
	Bomb->Destroy();
	if (bChain)
	{
		DetonateBomb(OtherIndex == 0 ? ERemoteBombShape::Sphere : ERemoteBombShape::Cube);
	}
}

void FRemoteBombAbility::ApplyExplosion(ARemoteBomb* Bomb, const FVector& Origin) const
{
	UWorld* World = Character ? Character->GetWorld() : nullptr;
	if (!World || !Bomb)
	{
		return;
	}
	const UGameConstantsDataAsset* Constants = UGameConstantsDataAsset::Get();
	FCollisionObjectQueryParams ObjectTypes;
	ObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
	ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
	FCollisionQueryParams OverlapParams(SCENE_QUERY_STAT(RemoteBombExplosion), false, Bomb);
	OverlapParams.AddIgnoredActor(Bomb);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, ObjectTypes, FCollisionShape::MakeSphere(Constants->RemoteBombExplosionRadius), OverlapParams);
	TSet<AActor*> DamagedActors;
	TSet<UPrimitiveComponent*> ImpulsedComponents;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		UPrimitiveComponent* Primitive = Overlap.GetComponent();
		if (!IsValid(Target) || Cast<ARemoteBomb>(Target))
		{
			continue;
		}
		if (Primitive && Primitive->IsSimulatingPhysics() && !ImpulsedComponents.Contains(Primitive))
		{
			Primitive->AddRadialImpulse(Origin, Constants->RemoteBombExplosionRadius, Constants->RemoteBombImpulse, ERadialImpulseFalloff::RIF_Constant, true);
			ImpulsedComponents.Add(Primitive);
		}
		if (DamagedActors.Contains(Target))
		{
			continue;
		}
		FCollisionQueryParams SightParams(SCENE_QUERY_STAT(RemoteBombOcclusion), false, Bomb);
		SightParams.AddIgnoredActor(Bomb);
		FHitResult SightHit;
		const FVector TargetLocation = Primitive ? Primitive->Bounds.Origin : Target->GetActorLocation();
		if (World->LineTraceSingleByChannel(SightHit, Origin, TargetLocation, ECC_Visibility, SightParams) && SightHit.GetActor() != Target)
		{
			continue;
		}
		DamagedActors.Add(Target);
		if (Target->CanBeDamaged())
		{
			UGameplayStatics::ApplyDamage(Target, Constants->RemoteBombDamage, Character->GetController(), Bomb, nullptr);
		}
		if (AKzPlayerCharacter* Player = Cast<AKzPlayerCharacter>(Target))
		{
			const FVector Direction = (Player->GetActorLocation() - Origin).GetSafeNormal();
			Player->LaunchCharacter(Direction * Constants->RemoteBombPlayerKnockback + FVector(0.0f, 0.0f, Constants->RemoteBombPlayerKnockback * 0.35f), false, false);
		}
	}
}

FVector FRemoteBombAbility::FindDropLocation(ARemoteBomb* Bomb) const
{
	UWorld* World = Character ? Character->GetWorld() : nullptr;
	if (!World || !Bomb)
	{
		return FVector::ZeroVector;
	}
	const float Radius = UGameConstantsDataAsset::Get()->RemoteBombRadius;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RemoteBombDrop), false, Character);
	QueryParams.AddIgnoredActor(Character);
	QueryParams.AddIgnoredActor(Bomb);
	for (int32 Step = 0; Step < 4; ++Step)
	{
		const FVector Horizontal = Character->GetActorLocation() + Character->GetActorForwardVector() * (Radius + 35.0f + Step * Radius);
		FHitResult GroundHit;
		const FVector TraceStart = Horizontal + FVector(0.0f, 0.0f, 80.0f);
		const FVector TraceEnd = Horizontal - FVector(0.0f, 0.0f, 220.0f);
		const FVector Candidate = World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams) ? GroundHit.ImpactPoint + FVector(0.0f, 0.0f, Radius + 3.0f) : Horizontal;
		if (!World->OverlapBlockingTestByChannel(Candidate, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), QueryParams))
		{
			return Candidate;
		}
	}
	return Character->GetActorLocation() + Character->GetActorForwardVector() * (Radius + 35.0f);
}

bool FRemoteBombAbility::CanPickUp(const ARemoteBomb* Bomb) const
{
	if (!Character || !Bomb || !Character->GetWorld())
	{
		return false;
	}
	const float Range = UGameConstantsDataAsset::Get()->RemoteBombPickupRange;
	if (FVector::DistSquared(Character->GetActorLocation(), Bomb->GetActorLocation()) > FMath::Square(Range))
	{
		return false;
	}
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RemoteBombPickup), false, Character);
	QueryParams.AddIgnoredActor(Character);
	FHitResult Hit;
	return !Character->GetWorld()->LineTraceSingleByChannel(Hit, Character->GetPawnViewLocation(), Bomb->GetActorLocation(), ECC_Visibility, QueryParams) || Hit.GetActor() == Bomb;
}

void FRemoteBombAbility::PlaceHeldBomb(const bool bThrow)
{
	ARemoteBomb* Bomb = HeldBomb.Get();
	if (!Bomb || !Character)
	{
		return;
	}
	const UGameConstantsDataAsset* Constants = UGameConstantsDataAsset::Get();
	const FVector Direction = Character->GetController() ? Character->GetController()->GetControlRotation().Vector() : Character->GetActorForwardVector();
	FVector Location = bThrow ? Bomb->GetActorLocation() + Direction * (Constants->RemoteBombRadius + 10.0f) : Bomb->GetActorLocation() + Character->GetActorForwardVector() * (Constants->RemoteBombRadius * 0.5f);
	FVector Impulse = bThrow ? (Direction + FVector(0.0f, 0.0f, 0.2f)).GetSafeNormal() * Constants->RemoteBombThrowImpulse : FVector::ZeroVector;
	if (bThrow)
	{
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RemoteBombThrow), false, Character);
		QueryParams.AddIgnoredActor(Character);
		QueryParams.AddIgnoredActor(Bomb);
		FHitResult Hit;
		if (Character->GetWorld()->SweepSingleByChannel(Hit, Bomb->GetActorLocation(), Location, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Constants->RemoteBombRadius), QueryParams))
		{
			Location = Hit.bStartPenetrating ? FindDropLocation(Bomb) : Hit.Location;
			Impulse = Hit.bStartPenetrating ? FVector::ZeroVector : Impulse;
		}
	}
	Bomb->Place(Location, Impulse);
	HeldBomb.Reset();
}
