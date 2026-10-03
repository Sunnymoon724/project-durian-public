#include "Framework/Movement/KZTitanClimbingComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/Skeleton.h"
#include "UObject/ConstructorHelpers.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "UObject/UnrealType.h"

namespace
{
	// Stop before the source clip's long standing hold, then blend to locomotion.
	constexpr float SwimmingExitEndTime = 1.5f;
	FVector SampleExitBone(UAnimSequence* Animation, FName Name, float Time)
	{
		const FReferenceSkeleton& Skeleton = Animation->GetSkeleton()->GetReferenceSkeleton();
		FTransform Result = FTransform::Identity;
		for (int32 Bone = Skeleton.FindBoneIndex(Name); Bone != INDEX_NONE; Bone = Skeleton.GetParentIndex(Bone))
		{
			FTransform Local = Skeleton.GetRefBonePose()[Bone];
			Animation->GetBoneTransform(Local, FSkeletonPoseBoneIndex(Bone), FAnimExtractContext(static_cast<double>(Time)), false);
			Result *= Local;
		}
		return Result.GetTranslation();
	}
}

UKzTitanClimbingComponent::UKzTitanClimbingComponent()
{
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ExitAnimation(
		TEXT("/Game/Resources/Soldier/Anims/Swim/anim_LedgeClimb_ClimbUp_Soldier"));
	SwimmingExitAnimation = ExitAnimation.Object;
}

bool UKzTitanClimbingComponent::StartClimbingOnTaggedWall()
{
	return !bSwimmingExit && bClimbTraceConfigured && StartClimbing();
}

bool UKzTitanClimbingComponent::TryStartSwimmingExit()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	if (!bClimbTraceConfigured || !Movement || !Movement->IsSwimming()
		|| bSwimmingExit || GetClimbState() != ETitanClimbState::Idle
		|| GetWorld()->GetTimeSeconds() < NextSwimmingExitTime
		|| (GetNetMode() != NM_Standalone && !Character->IsLocallyControlled()))
	{
		return false;
	}

	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const float Radius = Capsule->GetScaledCapsuleRadius();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FVector Start = Character->GetActorLocation();
	const FVector Forward = Character->GetActorForwardVector().GetSafeNormal2D();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SwimmingExit), false, Character);
	FHitResult Wall;
	if (!GetWorld()->LineTraceSingleByChannel(Wall, Start, Start + Forward * (Radius + 30.0f),
		ECC_GameTraceChannel1, Params) || !Wall.GetActor()
		|| !Wall.GetActor()->ActorHasTag(TEXT("Climbable"))
		|| FMath::Abs(Wall.ImpactNormal.Z) > 0.25f
		|| FVector::DotProduct(Forward, -Wall.ImpactNormal) < 0.8f)
	{
		return false;
	}

	const APhysicsVolume* Water = Character->GetPhysicsVolume();
	if (!Water || !Water->bWaterVolume)
	{
		return false;
	}
	FVector Origin, Extent;
	Water->GetActorBounds(false, Origin, Extent);
	const float SurfaceZ = Origin.Z + Extent.Z;
	// This is a short bank exit, not automatic climbing of an arbitrarily tall wall.
	const FVector LandingXY = Wall.ImpactPoint - Wall.ImpactNormal * (Radius + 12.0f);
	FHitResult Floor;
	if (!GetWorld()->LineTraceSingleByChannel(Floor,
		FVector(LandingXY.X, LandingXY.Y, SurfaceZ + 80.0f),
		FVector(LandingXY.X, LandingXY.Y, SurfaceZ - 20.0f), ECC_GameTraceChannel1, Params)
		|| !Movement->IsWalkable(Floor) || Floor.GetActor() != Wall.GetActor())
	{
		return false;
	}
	const FVector StandingLocation = Floor.ImpactPoint + FVector(0.0f, 0.0f, HalfHeight + 2.5f);
	if (GetWorld()->OverlapBlockingTestByChannel(StandingLocation, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeCapsule(Radius, HalfHeight), Params))
	{
		return false;
	}
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!SwimmingExitAnimation || !AnimInstance || SwimmingExitAnimation->GetPlayLength() < SwimmingExitEndTime
		|| Floor.ImpactPoint.Z - Start.Z > HalfHeight * 1.5f)
	{
		return false;
	}
	const FReferenceSkeleton& Skeleton = SwimmingExitAnimation->GetSkeleton()->GetReferenceSkeleton();
	if (Skeleton.FindBoneIndex(TEXT("root")) == INDEX_NONE
		|| Skeleton.FindBoneIndex(TEXT("hand_l")) == INDEX_NONE
		|| Skeleton.FindBoneIndex(TEXT("hand_r")) == INDEX_NONE)
	{
		return false;
	}

	SwimmingExitRotation = (-Wall.ImpactNormal).Rotation().Quaternion();
	const FQuat MeshRotation = SwimmingExitRotation * Character->GetMesh()->GetRelativeRotation().Quaternion();
	SwimmingExitRootStart = SampleExitBone(SwimmingExitAnimation, TEXT("root"), 0.0f);
	const FVector Hands = (SampleExitBone(SwimmingExitAnimation, TEXT("hand_l"), 0.0f)
		+ SampleExitBone(SwimmingExitAnimation, TEXT("hand_r"), 0.0f)) * 0.5f;
	const FVector HandOffset = MeshRotation.RotateVector(Hands - SwimmingExitRootStart)
		+ SwimmingExitRotation.RotateVector(Character->GetMesh()->GetRelativeLocation());
	SwimmingExitGrip = FVector(Wall.ImpactPoint.X, Wall.ImpactPoint.Y, Floor.ImpactPoint.Z) - HandOffset;
	SwimmingExitStart = Start;
	SwimmingExitSafeLocation = Start;
	const FVector RootEnd = SampleExitBone(SwimmingExitAnimation, TEXT("root"), SwimmingExitEndTime);
	SwimmingExitCorrection = StandingLocation - (SwimmingExitGrip + MeshRotation.RotateVector(RootEnd - SwimmingExitRootStart));
	SwimmingExitMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
		SwimmingExitAnimation, TEXT("DefaultSlot"), 0.18f, 0.2f);
	if (!SwimmingExitMontage)
	{
		return false;
	}
	// The animation's root trajectory is the only traversal clock. Extract it
	// from the pose, but do not let CharacterMovement apply it a second time.
	bSavedSwimmingOrientation = Movement->bOrientRotationToMovement;
	Movement->bOrientRotationToMovement = false;
	Movement->StopMovementImmediately();
	Movement->SetMovementMode(MOVE_Flying);
	Character->ConsumeMovementInputVector();
	SwimmingExitBank = Wall.GetActor();
	bBankWasIgnored = Character->GetCapsuleComponent()->GetMoveIgnoreActors().Contains(Wall.GetActor());
	// The capsule cannot fit through the hand-supported pose. Ignore only the
	// bank during this short mantle; keep sweeping against every other obstacle.
	Character->GetCapsuleComponent()->IgnoreActorWhenMoving(Wall.GetActor(), true);
	bSwimmingExit = true;
	SwimmingExitStartTime = GetWorld()->GetTimeSeconds();
	return true;
}

void UKzTitanClimbingComponent::CancelSwimmingExit()
{
	if (bSwimmingExit)
	{
		FinishSwimmingExit(false);
	}
}

void UKzTitanClimbingComponent::TickSwimmingExit()
{
	ACharacter* Character = CastChecked<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!SwimmingExitBank.IsValid() || !AnimInstance
		|| !AnimInstance->Montage_IsPlaying(SwimmingExitMontage)
		|| GetWorld()->GetTimeSeconds() - SwimmingExitStartTime > 4.0f)
	{
		FinishSwimmingExit(false);
		return;
	}
	const float Time = FMath::Min(AnimInstance->Montage_GetPosition(SwimmingExitMontage), SwimmingExitEndTime);
	const FQuat MeshRotation = SwimmingExitRotation * Character->GetMesh()->GetRelativeRotation().Quaternion();
	const FVector Root = SampleExitBone(SwimmingExitAnimation, TEXT("root"), Time);
	const float EntryBlend = FMath::SmoothStep(0.0f, 0.18f, Time);
	// Preserve the planted hands for the first pull. Align the standing root
	// only after the source animation releases its grip.
	const float LandingBlend = FMath::SmoothStep(0.55f, SwimmingExitEndTime, Time);
	const FVector Target = FMath::Lerp(SwimmingExitStart, SwimmingExitGrip, EntryBlend)
		+ MeshRotation.RotateVector(Root - SwimmingExitRootStart) + SwimmingExitCorrection * LandingBlend;
	Movement->StopMovementImmediately();
	Character->ConsumeMovementInputVector();
	FHitResult Hit;
	Character->SetActorLocationAndRotation(Target, SwimmingExitRotation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		FinishSwimmingExit(false);
		return;
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SwimmingExitClearance), false, Character);
	const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	const bool bClear = !GetWorld()->OverlapBlockingTestByChannel(Target, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()), Params);
	if (bClear)
	{
		SwimmingExitSafeLocation = Target;
	}
	if (Time >= SwimmingExitEndTime - KINDA_SMALL_NUMBER)
	{
		if (!bBankWasIgnored)
		{
			Character->GetCapsuleComponent()->IgnoreActorWhenMoving(SwimmingExitBank.Get(), false);
		}
		FFindFloorResult Floor;
		Movement->ComputeFloorDist(Target, 10.0f, 10.0f, Floor, Capsule->GetScaledCapsuleRadius());
		FinishSwimmingExit(bClear && Floor.IsWalkableFloor());
	}
}

void UKzTitanClimbingComponent::FinishSwimmingExit(bool bLanded)
{
	ACharacter* Character = CastChecked<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!bLanded)
	{
		Character->SetActorLocation(SwimmingExitSafeLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
	if (SwimmingExitBank.IsValid() && !bBankWasIgnored)
	{
		Character->GetCapsuleComponent()->IgnoreActorWhenMoving(SwimmingExitBank.Get(), false);
	}
	if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.2f, SwimmingExitMontage);
	}
	bSwimmingExit = false;
	SwimmingExitMontage = nullptr;
	SwimmingExitBank.Reset();
	NextSwimmingExitTime = GetWorld()->GetTimeSeconds() + 0.75f;
	Movement->bOrientRotationToMovement = bSavedSwimmingOrientation;
	Movement->StopMovementImmediately();
	Character->ConsumeMovementInputVector();
	Movement->SetMovementMode(bLanded ? MOVE_Walking
		: (Character->GetPhysicsVolume()->bWaterVolume ? MOVE_Swimming : MOVE_Falling));
}

void UKzTitanClimbingComponent::BeginPlay()
{
	Super::BeginPlay();

	const ETraceTypeQuery ClimbTraceType = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1);
	if (UCollisionProfile::Get()->ReturnChannelNameFromContainerIndex(ECC_GameTraceChannel1) != TEXT("Climbable") || ClimbTraceType == TraceTypeQuery_MAX)
	{
		UE_LOG(LogTemp, Error, TEXT("Climbable trace channel is missing; climbing is disabled."));
		return;
	}

	FByteProperty* TraceProperty = FindFProperty<FByteProperty>(UTitanClimbingComponent::StaticClass(), TEXT("ClimbableTraceType"));
	if (!TraceProperty)
	{
		UE_LOG(LogTemp, Error, TEXT("Titan Climbing trace channel could not be configured; climbing is disabled."));
		return;
	}

	TraceProperty->SetPropertyValue_InContainer(this, static_cast<uint8>(ClimbTraceType));
	bClimbTraceConfigured = true;

	for (TActorIterator<AActor> ActorIt(GetWorld()); ActorIt; ++ActorIt)
	{
		ConfigureClimbableCollision(*ActorIt);
	}
	ActorSpawnedHandle = GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &UKzTitanClimbingComponent::ConfigureClimbableCollision));
}

void UKzTitanClimbingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelSwimmingExit();
	if (ActorSpawnedHandle.IsValid() && GetWorld())
	{
		GetWorld()->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
		ActorSpawnedHandle.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

void UKzTitanClimbingComponent::ConfigureClimbableCollision(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	const ECollisionResponse Response = Actor->ActorHasTag(TEXT("Climbable")) ? ECR_Block : ECR_Ignore;
	TArray<UPrimitiveComponent*> Primitives;
	Actor->GetComponents(Primitives);
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		if (IsValid(Primitive))
		{
			Primitive->SetCollisionResponseToChannel(ECC_GameTraceChannel1, Response);
		}
	}
}

void UKzTitanClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	if (bSwimmingExit)
	{
		TickSwimmingExit();
		return;
	}
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	UCapsuleComponent* Capsule = Character ? Character->GetCapsuleComponent() : nullptr;
	// Match Titan's local simulation policy; remote transforms belong to Titan replication.
	const bool bCheckDescent = Character && Movement && Capsule
		&& (GetNetMode() == NM_Standalone || Character->IsLocallyControlled())
		&& GetClimbState() == ETitanClimbState::Climb
		&& GetAnimData().ClimbInput.Y < -KINDA_SMALL_NUMBER
		&& Capsule->IsQueryCollisionEnabled();
	const FVector Start = bCheckDescent ? Capsule->GetComponentLocation() : FVector::ZeroVector;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Jump is a press, not an auto-repeat: consume it once Titan builds the path.
	if (GetClimbState() == ETitanClimbState::Jump)
	{
		StopJumping();
	}

	// Do not interfere with entering, jumping, mantling, or Titan's normal walking exit.
	if (!bCheckDescent || GetClimbState() != ETitanClimbState::Climb)
	{
		return;
	}

	const FVector Down = Movement->GetGravityDirection();
	const float Descent = FVector::DotProduct(Capsule->GetComponentLocation() - Start, Down);
	if (Descent <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// Titan checks the OLD position, then applies the new anchor without a sweep.
	// Cover the whole movement, including a long frame that crosses the floor.
	// CharacterMovement's floor sweep handles adjacent walls, collision responses,
	// walkable slopes, and the capsule's actual dimensions (no world-Z clamp).
	const float FloorClearance = (UCharacterMovementComponent::MIN_FLOOR_DIST
		+ UCharacterMovementComponent::MAX_FLOOR_DIST) * 0.5f;
	const float ProbeDistance = Descent + UCharacterMovementComponent::MAX_FLOOR_DIST;
	FFindFloorResult Floor;
	Movement->ComputeFloorDist(Start, ProbeDistance, ProbeDistance, Floor,
		Capsule->GetScaledCapsuleRadius());
	if (!Floor.IsWalkableFloor() || Floor.GetDistanceToFloor() > Descent + FloorClearance)
	{
		return;
	}

	const FVector GroundedLocation = Start + Down * (Floor.GetDistanceToFloor() - FloorClearance);
	const FQuat UprightRotation = FRotationMatrix::MakeFromZX(-Down,
		Character->GetActorForwardVector()).ToQuat();
	StopClimbing();
	// Restore the last collision-tested horizontal position and put the capsule
	// on its supporting floor before CharacterMovement takes control again.
	Character->SetActorLocationAndRotation(GroundedLocation, UprightRotation, false, nullptr,
		ETeleportType::TeleportPhysics);
	Movement->StopMovementImmediately();
	Movement->SetMovementMode(MOVE_Walking);
}
