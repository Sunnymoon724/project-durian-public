#include "Abilities/RemoteBomb/RemoteBombAbility.h"

#include "DataAssets/GameConstantsDataAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

void URemoteBombAbility::SetBombClasses(TSubclassOf<ARemoteBomb> InSphereClass, TSubclassOf<ARemoteBomb> InCubeClass)
{
	BombClassArray[0] = InSphereClass;
	BombClassArray[1] = InCubeClass;
}

void URemoteBombAbility::Tick(float)
{
	if (HeldBomb.IsValid() && (!Character || !Character->GetCharacterMovement() || Character->GetCharacterMovement()->IsFalling() || Character->IsWallClimbing()))
	{
		DropHeldBomb();
	}
	else if (bThrowPending && HeldBomb.IsValid())
	{
		if (!Character->IsBombThrowAnimationPlaying())
		{
			DropHeldBomb();
		}
		else if (Character->HasBombThrowReachedRelease())
		{
			PlaceHeldBomb(true);
		}
	}
}

void URemoteBombAbility::HandleInput(const EAbilityInput Input, float)
{
	if (!Character)
	{
		return;
	}

	switch (Input)
	{
	case EAbilityInput::Interact:
		{

			if (HeldBomb.IsValid())
			{
				PlaceHeldBomb(false);
				return;
			}
			{
				ERemoteBombShape Shape;
				if (!GetSelectedShape(Shape))
				{
					return;
				}
				ARemoteBomb* Bomb = BombArray[ToIndex(Shape)].Get();
				if (Bomb && !Bomb->IsHeld() && CanPickUp(Bomb))
				{
					Bomb->Hold(UGameConstantsDataAsset::Get()->RemoteBombHoldHeight);
					HeldBomb = Bomb;
				}
			}
			break;
		}
	case EAbilityInput::Cancel:
		{
			if (ARemoteBomb* Bomb = HeldBomb.Get())
			{
				BombArray[ToIndex(Bomb->GetShape())].Reset();
				HeldBomb.Reset();
				bThrowPending = false;
				Character->StopBombCarryAnimation();
				Bomb->Destroy();
			}
			break;
		}
	case EAbilityInput::Interrupt:
		DropHeldBomb();
		break;
	case EAbilityInput::Use:
		{
			ERemoteBombShape Shape;

			if (!GetSelectedShape(Shape) || Character->GetCurrentState() != EPlayerState::Normal || HeldBomb.IsValid())
			{
				return;
			}

			const int32 Index = ToIndex(Shape);
			const ARemoteBomb* CurrentBomb = BombArray[Index].Get();

			if (CurrentBomb)
			{
				if (!CurrentBomb->IsHeld())
				{
					DetonateBomb(Shape);
				}

				return;
			}

			if (GetCooldownRemaining(Shape) <= 0.0f)
			{
				SpawnBomb(Shape);
			}

			break;
		}
	case EAbilityInput::RemoteBombThrow:
		{
			ERemoteBombShape Shape;

			if (GetSelectedShape(Shape) && HeldBomb.IsValid() && HeldBomb->GetShape() == Shape && !bThrowPending)
			{
				bThrowPending = Character->PlayBombThrowAnimation();
			}

			break;
		}
	default:
		break;
	}
}

void URemoteBombAbility::OnDeselected()
{
	DropHeldBomb();
}

void URemoteBombAbility::DropHeldBomb()
{
	if (Character) Character->StopBombCarryAnimation();
	PlaceHeldBomb(false);
	bThrowPending = false;
}

void URemoteBombAbility::AbortForEndPlay()
{
	HeldBomb.Reset();
	bThrowPending = false;

	for (int32 Index = 0; Index < 2; ++Index)
	{
		if (ARemoteBomb* Bomb = BombArray[Index].Get())
		{
			Bomb->Destroy();
		}

		BombArray[Index].Reset();
		CooldownEndTimeArray[Index] = 0.0;
	}
}

float URemoteBombAbility::GetCooldownRemaining(const ERemoteBombShape Shape) const
{
	return GetRemainingCooldown(CooldownEndTimeArray[ToIndex(Shape)]);
}

bool URemoteBombAbility::HasBomb(const ERemoteBombShape Shape) const
{
	return BombArray[ToIndex(Shape)].IsValid();
}

bool URemoteBombAbility::IsBombInstalled(const ERemoteBombShape Shape) const
{
	const ARemoteBomb* Bomb = BombArray[ToIndex(Shape)].Get();

	return Bomb && !Bomb->IsHeld();
}

bool URemoteBombAbility::GetSelectedShape(ERemoteBombShape& OutShape) const
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

void URemoteBombAbility::SpawnBomb(const ERemoteBombShape Shape)
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

	const int Index = ToIndex(Shape);
	const TSubclassOf<ARemoteBomb> BombClass = BombClassArray[Index];

	if (!BombClass)
	{
		UE_LOG(LogTemp, Error, TEXT("RemoteBomb class for shape %d is not assigned on the player."), static_cast<int32>(Shape));

		return;
	}

	ARemoteBomb* NewBomb = World->SpawnActor<ARemoteBomb>(BombClass, HoldLocation, FRotator::ZeroRotator, Params);

	if (!NewBomb)
	{
		return;
	}

	NewBomb->Hold(Constants->RemoteBombHoldHeight);

	BombArray[Index] = NewBomb;
	HeldBomb = NewBomb;
}

void URemoteBombAbility::DetonateBomb(const ERemoteBombShape Shape)
{
	const int32 Index = ToIndex(Shape);
	ARemoteBomb* Bomb = BombArray[Index].Get();

	if (!Bomb || Bomb->IsHeld())
	{
		return;
	}
	const UGameConstantsDataAsset* Constants = UGameConstantsDataAsset::Get();
	const float ExplosionRadius = Constants->RemoteBombExplosionRadius;
	const FVector Origin = Bomb->GetActorLocation();
	const int32 OtherIndex = 1 - Index;
	const ARemoteBomb* OtherBomb = BombArray[OtherIndex].Get();
	const bool bChain = OtherBomb && !OtherBomb->IsHeld() && FVector::DistSquared(Origin, OtherBomb->GetActorLocation()) <= FMath::Square(ExplosionRadius);
	BombArray[Index].Reset();
	CooldownEndTimeArray[Index] = Character && Character->GetWorld() ? Character->GetWorld()->GetTimeSeconds() + Constants->RemoteBombCooldown : 0.0;
	Bomb->PlayExplosionEffect(ExplosionRadius);
	ApplyExplosion(Bomb, Origin, ExplosionRadius);
	Bomb->Destroy();

	if (bChain)
	{
		DetonateBomb(OtherIndex == 0 ? ERemoteBombShape::Sphere : ERemoteBombShape::Cube);
	}
}

void URemoteBombAbility::ApplyExplosion(ARemoteBomb* Bomb, const FVector& Origin, const float Radius) const
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
	World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, ObjectTypes, FCollisionShape::MakeSphere(Radius), OverlapParams);
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
			Primitive->AddRadialImpulse(Origin, Radius, Constants->RemoteBombImpulse, ERadialImpulseFalloff::RIF_Constant, true);
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

FVector URemoteBombAbility::FindDropLocation(const ARemoteBomb* Bomb) const
{
	const UWorld* World = Character ? Character->GetWorld() : nullptr;

	if (!World || !Bomb)
	{
		return FVector::ZeroVector;
	}

	const float Radius = UGameConstantsDataAsset::Get()->RemoteBombRadius;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RemoteBombDrop), false, Character);
	QueryParams.AddIgnoredActor(Character);
	QueryParams.AddIgnoredActor(Bomb);

	// The overhead carry keeps the cube upright. Leave clearance for its full
	// collision bounds, not just the nominal bomb radius, when placing it.
	const UPrimitiveComponent* BombRoot = Cast<UPrimitiveComponent>(Bomb->GetRootComponent());
	const float Clearance = Character->GetCapsuleComponent()->GetScaledCapsuleRadius()
		+ FMath::Max(Radius, BombRoot ? static_cast<float>(BombRoot->Bounds.SphereRadius) : Radius) + 5.0f;
	const float DropDistance = FMath::Max(Radius + 35.0f, Clearance);
	for (int32 Step = 0; Step < 4; ++Step)
	{
		const FVector Horizontal = Character->GetActorLocation() + Character->GetActorForwardVector() * (DropDistance + Step * Radius);
		FHitResult GroundHit;
		const FVector TraceStart = Horizontal + FVector(0.0f, 0.0f, 80.0f);
		const FVector TraceEnd = Horizontal - FVector(0.0f, 0.0f, 220.0f);
		const FVector Candidate = World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams) ? GroundHit.ImpactPoint + FVector(0.0f, 0.0f, Radius + 3.0f) : Horizontal;
		if (!World->OverlapBlockingTestByChannel(Candidate, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius), QueryParams))
		{
			return Candidate;
		}
	}

	return Character->GetActorLocation() + Character->GetActorForwardVector() * DropDistance;
}

bool URemoteBombAbility::CanPickUp(const ARemoteBomb* Bomb) const
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

void URemoteBombAbility::PlaceHeldBomb(const bool bThrow)
{
	ARemoteBomb* Bomb = HeldBomb.Get();

	if (!Bomb || !Character)
	{
		return;
	}

	const UGameConstantsDataAsset* Constants = UGameConstantsDataAsset::Get();
	const FVector Direction = Character->GetController() ? Character->GetController()->GetControlRotation().Vector() : Character->GetActorForwardVector();

	FVector Location = bThrow ? Bomb->GetActorLocation() : FindDropLocation(Bomb);
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
	if (!bThrow) Character->StopBombCarryAnimation();
	HeldBomb.Reset();
	bThrowPending = false;
}
