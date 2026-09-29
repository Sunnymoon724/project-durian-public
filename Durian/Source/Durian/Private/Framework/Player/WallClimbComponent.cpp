#include "Framework/Player/WallClimbComponent.h"

#include "Framework/Player/KzPlayerCharacter.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

UWallClimbComponent::UWallClimbComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> VaultFinder(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/M_Neutral_Traversal_Catch_Mantle_med_stand_SoldierFixed"));
	VaultSequence = VaultFinder.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> TallWallFinder(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Up"));
	TallWallClimbSequence = TallWallFinder.Object;
}

void UWallClimbComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWallClimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	if (bClimbing)
	{
		// CharacterMovement must not apply gravity or input on top of the
		// component-controlled vault path.
		Character->GetCharacterMovement()->StopMovementImmediately();
		ElapsedClimbTime += DeltaTime;
		const float Alpha = FMath::Clamp(ElapsedClimbTime / ActiveClimbDuration, 0.f, 1.f);
		// A tall climb reaches the lip during the climb pose. The capsule must
		// finish crossing the lip before the animation settles into standing.
		const float RiseStart = bTallWallClimb ? 0.12f : 0.f;
		const float RiseEnd = bTallWallClimb ? 0.68f : 0.30f;
		const float MoveEnd = bTallWallClimb ? 0.85f : 0.50f;
		const float SettleEnd = bTallWallClimb ? 0.94f : 0.62f;
		FVector NextLocation;
		if (Alpha < RiseStart)
		{
			NextLocation = ClimbStart;
		}
		else if (Alpha < RiseEnd)
		{
			NextLocation = FMath::Lerp(ClimbStart, ClimbApex,
				FMath::InterpEaseInOut(0.f, 1.f, (Alpha - RiseStart) / (RiseEnd - RiseStart), 2.f));
		}
		else if (Alpha < MoveEnd)
		{
			FVector TopPosition = ClimbTarget;
			TopPosition.Z = ClimbApex.Z;
			NextLocation = FMath::Lerp(ClimbApex, TopPosition,
				FMath::InterpEaseInOut(0.f, 1.f, (Alpha - RiseEnd) / (MoveEnd - RiseEnd), 2.f));
		}
		else if (Alpha < SettleEnd)
		{
			FVector TopPosition = ClimbTarget;
			TopPosition.Z = ClimbApex.Z;
			NextLocation = FMath::Lerp(TopPosition, ClimbTarget,
				FMath::InterpEaseInOut(0.f, 1.f, (Alpha - MoveEnd) / (SettleEnd - MoveEnd), 2.f));
		}
		else
		{
			NextLocation = ClimbTarget;
		}
		FHitResult MoveHit;
		// Never teleport through a wall.  If the capsule cannot follow this route,
		// abort the climb at the last valid position instead of forcing it through.
		if (Character->SetActorLocation(NextLocation, true, &MoveHit) && !MoveHit.bBlockingHit)
		{
			if (Alpha >= 1.f)
			{
				FinishWallClimb();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SoldierVault blocked by %s at %s"),
				*GetNameSafe(MoveHit.GetActor()), *MoveHit.ImpactPoint.ToCompactString());
			FinishWallClimb(false);
		}
		return;
	}
	if (ClimbRetryCooldown > 0.f)
	{
		ClimbRetryCooldown = FMath::Max(0.f, ClimbRetryCooldown - DeltaTime);
		return;
	}
	if (const AKzPlayerCharacter* Soldier = Cast<AKzPlayerCharacter>(Character);
		Soldier && Soldier->IsHovering())
	{
		return;
	}

	// A short mantle begins only after a regular jump reaches a nearby ledge.
	// The separate high-wall climb is started by a grounded Space press.
	if (Character->GetCharacterMovement()->IsFalling()
		&& Character->GetCharacterMovement()->Velocity.Z <= 0.f)
	{
		// Titan releases at the lip and immediately switches the movement mode
		// to Falling. Start the scripted top-out on that first falling frame.
		if (!TryWallClimb())
		{
			TryStartTallWallClimb();
		}
	}
}

bool UWallClimbComponent::TryWallClimb()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	const AKzPlayerCharacter* Soldier = Cast<AKzPlayerCharacter>(Character);
	if (!Soldier || Soldier->GetForwardInputValue() <= 0.1f
		|| !Character->GetCharacterMovement()->IsFalling()
		|| Character->GetCharacterMovement()->Velocity.Z > 0.f)
	{
		return false;
	}

	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
	const float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float FootHeight = Character->GetActorLocation().Z - CapsuleHalfHeight;
	// Trace slightly below the feet so it still hits a wall as the jump reaches
	// its top, instead of tracing over the lip.
	const FVector TraceStart = Character->GetActorLocation() + FVector(0.f, 0.f, -CapsuleHalfHeight - 30.f);
	const FVector TraceEnd = TraceStart + Forward * 140.f;
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SoldierWallClimb), false, Character);
	if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params) || !Hit.GetActor())
	{
		return false;
	}

	FVector BoundsOrigin;
	FVector BoundsExtent;
	Hit.GetActor()->GetActorBounds(false, BoundsOrigin, BoundsExtent);
	const float WallHeight = BoundsExtent.Z * 2.f;
	if (WallHeight < MinimumWallHeight || WallHeight > MaximumWallHeight)
	{
		return false;
	}

	const float WallTop = BoundsOrigin.Z + BoundsExtent.Z;
	// A surface below the jumping feet is the floor, not a ledge to mantle.
	if (WallTop - FootHeight < -10.f || WallTop - FootHeight > 70.f)
	{
		return false;
	}

	// Preserve the contact point along the wall, then move only to its top.
	FVector Target = Hit.ImpactPoint;
	Target.Z = WallTop + CapsuleHalfHeight + 2.f;
	const float DistanceToWallCentre = FVector::DotProduct(BoundsOrigin - Hit.ImpactPoint, Forward);
	Target += Forward * FMath::Clamp(DistanceToWallCentre, 0.f, LandingForwardOffset);
	FVector Apex = Character->GetActorLocation();
	Apex.Z = WallTop + CapsuleHalfHeight + 15.f;
	if (!StartClimb(Target, Apex, VaultSequence, ClimbDuration, false))
	{
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("SoldierVault start: wall=%s, top target=%s"),
		*GetNameSafe(Hit.GetActor()), *Target.ToCompactString());
	return true;
}

bool UWallClimbComponent::TryStartTallWallClimb()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	const AKzPlayerCharacter* Soldier = Cast<AKzPlayerCharacter>(Character);
	if (!Soldier || bClimbing || ClimbRetryCooldown > 0.f
		|| Soldier->GetForwardInputValue() <= 0.1f
		|| !Character->GetCharacterMovement()->IsFalling()
		|| Character->GetCharacterMovement()->Velocity.Z > 0.f)
	{
		return false;
	}

	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
	const float CapsuleRadius = Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector TraceStart = Character->GetActorLocation();
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SoldierTallWallClimb), false, Character);
	if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart,
		TraceStart + Forward * (CapsuleRadius + 35.f), ECC_Visibility, Params)
		|| !Hit.GetActor() || FVector::DotProduct(Hit.ImpactNormal, Forward) > -0.7f)
	{
		return false;
	}

	FVector BoundsOrigin;
	FVector BoundsExtent;
	Hit.GetActor()->GetActorBounds(false, BoundsOrigin, BoundsExtent);
	const float WallTop = BoundsOrigin.Z + BoundsExtent.Z;
	const float FootHeight = Character->GetActorLocation().Z - CapsuleHalfHeight;
	const float RemainingHeight = WallTop - FootHeight;
	if (RemainingHeight <= 70.f)
	{
		return false;
	}

	const float DistanceToWallCentre = FVector::DotProduct(BoundsOrigin - Hit.ImpactPoint, Forward);
	FVector Target = Hit.ImpactPoint + Forward * FMath::Clamp(DistanceToWallCentre, 0.f, LandingForwardOffset);
	Target.Z = WallTop + CapsuleHalfHeight + 2.f;
	FVector Apex = Hit.ImpactPoint - Forward * (CapsuleRadius + 3.f);
	Apex.Z = WallTop + CapsuleHalfHeight + 15.f;
	// The remaining ascent distance controls the duration; a 300 cm wall
	// therefore cannot complete at the same speed as a 200 cm wall.
	const float Duration = FMath::Max(1.8f, TallWallClimbDuration * RemainingHeight / 200.f);
	// Do not force a full-body sequence here. The imported Titan clips use a
	// different reference pose and deform the Soldier when played directly.
	if (!StartClimb(Target, Apex, nullptr, Duration, true))
	{
		return false;
	}
	UE_LOG(LogTemp, Log, TEXT("SoldierTallWallClimb start: wall=%s, top target=%s"),
		*GetNameSafe(Hit.GetActor()), *Target.ToCompactString());
	return true;
}

bool UWallClimbComponent::TryFinishTitanClimb()
{
	return TryWallClimb() || TryStartTallWallClimb();
}

bool UWallClimbComponent::CancelTallWallClimb()
{
	if (!bClimbing || !bTallWallClimb)
	{
		return false;
	}
	FinishWallClimb(false);
	return true;
}

bool UWallClimbComponent::StartClimb(const FVector& Target, const FVector& Apex,
	UAnimSequenceBase* Sequence, float Duration, bool bTallWall)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || Duration <= 0.f || bClimbing)
	{
		return false;
	}
	ClimbStart = Character->GetActorLocation();
	ClimbTarget = Target;
	ClimbApex = Apex;
	ActiveClimbDuration = Duration;
	bTallWallClimb = bTallWall;
	ElapsedClimbTime = 0.f;
	bClimbing = true;
	// The Soldier sequence plays through a single-node instance, so the ABP
	// does not need a movement state during this scripted movement.
	Character->GetCharacterMovement()->DisableMovement();
	Character->GetCharacterMovement()->StopMovementImmediately();

	if (Sequence)
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			SavedAnimInstanceClass = Mesh->GetAnimInstance() ? Mesh->GetAnimInstance()->GetClass() : nullptr;
			Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			if (UAnimSingleNodeInstance* SingleNode = Mesh->GetSingleNodeInstance())
			{
				const float PlaybackLength = bTallWall ? FMath::Min(2.4f, Sequence->GetPlayLength()) : Sequence->GetPlayLength();
				SingleNode->SetAnimationAsset(Sequence, false, PlaybackLength / Duration);
				SingleNode->SetPosition(0.f, false);
				SingleNode->SetPlaying(true);
			}
		}
	}
	return true;
}

void UWallClimbComponent::FinishWallClimb(bool bReachedTop)
{
	bClimbing = false;
	bTallWallClimb = false;
	ClimbRetryCooldown = 0.5f;
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (SavedAnimInstanceClass)
			{
				Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				Mesh->SetAnimInstanceClass(SavedAnimInstanceClass);
			}
		}
		SavedAnimInstanceClass = nullptr;
		Character->GetCharacterMovement()->StopMovementImmediately();
		Character->GetCharacterMovement()->SetMovementMode(bReachedTop ? MOVE_Walking : MOVE_Falling);
	}
}


