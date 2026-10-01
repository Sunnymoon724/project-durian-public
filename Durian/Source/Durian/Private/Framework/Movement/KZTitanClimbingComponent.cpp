#include "Framework/Movement/KZTitanClimbingComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/UnrealType.h"

bool UKzTitanClimbingComponent::StartClimbingOnTaggedWall()
{
	return bClimbTraceConfigured && StartClimbing();
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
