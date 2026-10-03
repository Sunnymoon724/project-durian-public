// ReSharper disable All
#include "Abilities/Magnesis/MagnesisAbility.h"

#include "Abilities/Components/AbilityReactionComponent.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "CollisionShape.h"
#include "DataAssets/GameConstantsDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Enums/PlayerEnums.h"
#include "Engine/World.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Utilities/NiagaraEffectUtility.h"
#include "NiagaraComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

void UMagnesisAbility::Initialize(AKzPlayerCharacter* InCharacter)
{
	Super::Initialize(InCharacter);

	const UGameConstantsDataAsset* Constants = UGameConstantsDataAsset::Get();

	MagnesisDistance = Constants->MagnesisDefaultDistance;

	if (Character)
	{
		PhysicsHandle = NewObject<UPhysicsHandleComponent>(Character, TEXT("MagnesisPhysicsHandle"));

		Character->AddInstanceComponent(PhysicsHandle);
		PhysicsHandle->RegisterComponent();

		PhysicsHandle->LinearStiffness = Constants->MagnesisPhysicsHandleLinearStiffness;
		PhysicsHandle->LinearDamping = Constants->MagnesisPhysicsHandleLinearDamping;
		PhysicsHandle->AngularStiffness = Constants->MagnesisPhysicsHandleAngularStiffness;
		PhysicsHandle->AngularDamping = Constants->MagnesisPhysicsHandleAngularDamping;
	}
}

void UMagnesisAbility::Tick(float DeltaTime)
{
	if (!Character)
	{
		return;
	}

	EPlayerState state = Character->GetCurrentState();

	if ((state == EPlayerState::MagnesisTargeting || state == EPlayerState::MagnesisHolding) && Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsFalling())
	{
		Release();

		return;
	}

	switch (state)
	{
	case EPlayerState::MagnesisTargeting:
		UpdateTargeting();
		break;
	case EPlayerState::MagnesisHolding:
		UpdateControl(DeltaTime);
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("Not Supported in %s."), *UEnum::GetValueAsString(state));
		break;
	}
}

void UMagnesisAbility::HandleInput(const EAbilityInput Input, const float AxisValue)
{
	if (!Character)
	{
		return;
	}

	switch (Input)
	{
	case EAbilityInput::Interact:
		{
			if (Character->GetCurrentState() == EPlayerState::MagnesisTargeting && Character->IsScanPoseReady())
			{
				SelectTarget();
			}
			break;
		}
	case EAbilityInput::Cancel:
	case EAbilityInput::Interrupt:
		{
			Release();
			break;
		}
	case EAbilityInput::Use:
		{
			if (Character->GetCurrentState() == EPlayerState::Normal)
			{
				EnterTargetingMode();
			}
			else
			{
				Release();
			}
			break;
		}
	case EAbilityInput::MagnesisDistance:
		{
			if (Character->GetCurrentState() == EPlayerState::MagnesisHolding && !FMath::IsNearlyZero(AxisValue))
			{
				MagnesisDistance = FMath::Clamp(MagnesisDistance + AxisValue * 100.0f, UGameConstantsDataAsset::Get()->MagnesisMinDistance, UGameConstantsDataAsset::Get()->MagnesisMaxDistance);
			}
			break;
		}
	default:
		UE_LOG(LogTemp, Log, TEXT("Unsupported Magnesis input: %d."), static_cast<int32>(Input));
		break;
	}
}

void UMagnesisAbility::EnterTargetingMode()
{
	if (!Character)
	{
		return;
	}

	Character->SetPlayerState(EPlayerState::MagnesisTargeting);

}

void UMagnesisAbility::UpdateTargeting()
{
	if (!UpdateScanActivation(EAbilityVisualMode::Magnesis, EAbilityType::Magnesis, true))
	{
		ClearTargetedComponent();
		return;
	}
	UPrimitiveComponent* HitComponent = nullptr;
	FVector HitLocation = FVector::ZeroVector;

	if (TraceTarget(HitComponent, HitLocation))
	{
		SetTargetedComponent(HitComponent, HitLocation);

		return;
	}

	ClearTargetedComponent();
}

void UMagnesisAbility::UpdateControl(const float DeltaTime)
{
	UPhysicsHandleComponent* MagnesisPhysicsHandle = PhysicsHandle;

	if (!Character || !MagnesisPhysicsHandle || !Character->GetController())
	{
		Release();

		return;
	}

	if (!MagnesisPhysicsHandle->GetGrabbedComponent())
	{
		RestoreHeldPawnCollision();

		if (UPlayerAbilityComponent* AbilityComponent = Character->GetAbilityComponent())
		{
			AbilityComponent->SetMagnesisHeldActor(nullptr);
		}

		StopHoldBeam();
		Character->SetPlayerState(EPlayerState::Normal);

		return;
	}

	const FVector HoldLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
	// Use the controller view (mouse) direction, not the pawn body's forward
	// direction, so a held object follows camera yaw and pitch.
	
	const FVector TargetLocation = HoldLocation + Character->GetController()->GetControlRotation().Vector() * MagnesisDistance;
	FVector SafeTargetLocation = TargetLocation;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MagnesisTargetSweep), false, Character);

	QueryParams.AddIgnoredActor(MagnesisPhysicsHandle->GetGrabbedComponent()->GetOwner());

	FHitResult SweepHit;

	if (Character->GetWorld()->SweepSingleByChannel(SweepHit, HoldLocation, TargetLocation, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(40.0f), QueryParams))
	{
		SafeTargetLocation = SweepHit.Location;
	}

	// Move the physics-handle target gradually so the object follows with a soft lag.
	CurrentHoldLocation = FMath::VInterpTo(CurrentHoldLocation, SafeTargetLocation, DeltaTime, UGameConstantsDataAsset::Get()->MagnesisFollowSpeed);

	MagnesisPhysicsHandle->SetTargetLocation(CurrentHoldLocation);

	UpdateHoldBeam();
}

void UMagnesisAbility::SelectTarget()
{
	UPhysicsHandleComponent* MagnesisPhysicsHandle = PhysicsHandle;
	UPrimitiveComponent* HitComponent = nullptr;
	FVector HitLocation = FVector::ZeroVector;

	if (!Character || !MagnesisPhysicsHandle)
	{
		return;
	}

	if (!TraceTarget(HitComponent, HitLocation))
	{
		ExitTargetingMode();

		Character->SetPlayerState(EPlayerState::Normal);

		return;
	}

	MagnesisPhysicsHandle->GrabComponentAtLocation(HitComponent, NAME_None, HitLocation);
	ExitTargetingMode();

	if (MagnesisPhysicsHandle->GetGrabbedComponent())
	{
		UPrimitiveComponent* GrabbedComponent = MagnesisPhysicsHandle->GetGrabbedComponent();
		HeldComponent = GrabbedComponent;
		HeldPawnCollisionResponse = GrabbedComponent->GetCollisionResponseToChannel(ECC_Pawn);
		GrabbedComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		if (UPlayerAbilityComponent* AbilityComponent = Character->GetAbilityComponent())
		{
			AbilityComponent->SetMagnesisHeldActor(MagnesisPhysicsHandle->GetGrabbedComponent()->GetOwner());
		}

		if (USkeletalMeshComponent* CharacterMesh = Character->GetMesh())
		{
			HoldBeam = FNiagaraEffectUtility::SpawnAttachedRelative(CharacterMesh, FSoftObjectPath(TEXT("/Game/Resources/VFX/Magnesis/Niagara/NS_MagnesisHoldBeam.NS_MagnesisHoldBeam")), TEXT("hand_r"), FVector::ZeroVector, FRotator::ZeroRotator, true, false);

			if (UNiagaraComponent* Beam = HoldBeam.Get())
			{
				// Follow the hand position while keeping the field's up axis in world space.
				Beam->SetAbsolute(false, true, true);
				Beam->SetWorldRotation(FRotator::ZeroRotator);
				Beam->SetWorldScale3D(FVector::OneVector);
				Beam->SetRelativeLocation(FVector::ZeroVector);
			}
		}

		CurrentHoldLocation = HitLocation;

		const FVector HoldLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);

		MagnesisDistance = FMath::Clamp(FVector::DotProduct(HitLocation - HoldLocation, Character->GetController()->GetControlRotation().Vector()), UGameConstantsDataAsset::Get()->MagnesisMinDistance, UGameConstantsDataAsset::Get()->MagnesisMaxDistance);

		Character->SetPlayerState(EPlayerState::MagnesisHolding);

		UpdateHoldBeam();
		if (UNiagaraComponent* Beam = HoldBeam.Get())
		{
			// Set the target before the first frame of the persistent beam.
			Beam->Activate(true);
		}
	}
	else
	{
		Character->SetPlayerState(EPlayerState::Normal);
	}
}

void UMagnesisAbility::ExitTargetingMode()
{
	if (!Character)
	{
		return;
	}

	ClearTargetedComponent();

	SetAbilityModeActive(EAbilityVisualMode::Magnesis, false);
	ClearAbilityVisionEffects();
}

bool UMagnesisAbility::TraceTarget(UPrimitiveComponent*& OutComponent, FVector& OutLocation) const
{
	OutComponent = nullptr;
	OutLocation = FVector::ZeroVector;

	FHitResult HitResult;
	const bool bValidTarget = TraceAbilityTarget(EAbilityReactionType::MagnesisTarget, UGameConstantsDataAsset::Get()->MagnesisTargetRange, HitResult, false, nullptr, true);
	const bool bTraceHit = HitResult.bBlockingHit;

	if (!bTraceHit || !bValidTarget)
	{
		return false;
	}

	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	OutComponent = HitComponent;
	OutLocation = HitResult.ImpactPoint;

	return true;
}

void UMagnesisAbility::Release()
{
	if (UPhysicsHandleComponent* MagnesisPhysicsHandle = PhysicsHandle)
	{
		if (MagnesisPhysicsHandle->GetGrabbedComponent())
		{
			MagnesisPhysicsHandle->ReleaseComponent();
		}
	}

	RestoreHeldPawnCollision();

	if (!Character)
	{
		return;
	}

	if (UPlayerAbilityComponent* AbilityComponent = Character->GetAbilityComponent())
	{
		AbilityComponent->SetMagnesisHeldActor(nullptr);
	}

	StopHoldBeam();
	ClearTargetedComponent();

	CurrentHoldLocation = FVector::ZeroVector;
	Character->SetPlayerState(EPlayerState::Normal);

	SetAbilityModeActive(EAbilityVisualMode::Magnesis, false);
	ClearAbilityVisionEffects();
}

void UMagnesisAbility::RestoreHeldPawnCollision()
{
	if (UPrimitiveComponent* Component = HeldComponent.Get())
	{
		Component->SetCollisionResponseToChannel(ECC_Pawn, HeldPawnCollisionResponse);
	}

	HeldComponent.Reset();
}

void UMagnesisAbility::UpdateHoldBeam()
{
	const UPrimitiveComponent* GrabbedComponent = PhysicsHandle ? PhysicsHandle->GetGrabbedComponent() : nullptr;

	if (HoldBeam.IsValid() && GrabbedComponent)
	{
		if (Character && Character->GetMesh())
		{
			HoldBeam->SetWorldLocation(Character->GetMesh()->GetSocketLocation(TEXT("hand_r")));
		}
		// The approved Niagara system expects a component-local target position.
		const FVector LocalTarget = HoldBeam->GetComponentTransform().InverseTransformPosition(GrabbedComponent->Bounds.Origin);
		HoldBeam->SetVariablePosition(TEXT("User.TargetPosition"), LocalTarget);
	}
}

void UMagnesisAbility::StopHoldBeam()
{
	if (UNiagaraComponent* Beam = HoldBeam.Get())
	{
		Beam->DeactivateImmediate();
		Beam->DestroyComponent();
	}

	HoldBeam.Reset();
}

void UMagnesisAbility::SetTargetedComponent(UPrimitiveComponent* NewTarget, const FVector& NewTargetLocation)
{
	if (TargetedComponent.IsValid() && TargetedComponent.Get() != NewTarget)
	{
		TargetedComponent->SetCustomDepthStencilValue(1);

		if (UAbilityReactionComponent* PreviousReaction = TargetedComponent->GetOwner() ? TargetedComponent->GetOwner()->FindComponentByClass<UAbilityReactionComponent>() : nullptr)
		{
			PreviousReaction->SetAimedTarget(false);
		}
	}

	TargetedComponent = NewTarget;
	TargetedLocation = NewTargetLocation;

	if (NewTarget)
	{
		NewTarget->SetRenderCustomDepth(true);
		NewTarget->SetCustomDepthStencilValue(2);

		if (UAbilityReactionComponent* Reaction = NewTarget->GetOwner() ? NewTarget->GetOwner()->FindComponentByClass<UAbilityReactionComponent>() : nullptr)
		{
			Reaction->SetAimedTarget(true);
		}
	}
}

void UMagnesisAbility::ClearTargetedComponent()
{
	if (TargetedComponent.IsValid())
	{
		TargetedComponent->SetCustomDepthStencilValue(1);

		if (UAbilityReactionComponent* Reaction = TargetedComponent->GetOwner()? TargetedComponent->GetOwner()->FindComponentByClass<UAbilityReactionComponent>() : nullptr)
		{
			Reaction->SetAimedTarget(false);
		}
	}

	TargetedComponent.Reset();
	TargetedLocation = FVector::ZeroVector;
}
