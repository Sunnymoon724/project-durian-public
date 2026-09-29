// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Player/KzPlayerCharacter.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Components/DamageableComponent.h"
#include "Abilities/Core/Ability.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TitanClimbing.h"
#include "TitanClimbingComponent.h"

AKzPlayerCharacter::AKzPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilityEffect = CreateDefaultSubobject<UAbilityEffectComponent>(TEXT("AbilityEffect"));

	Damageable = CreateDefaultSubobject<UDamageableComponent>(TEXT("Damageable"));
	Damageable->SetDestroyOwnerOnDepleted(false);

	AbilityComponent = CreateDefaultSubobject<UPlayerAbilityComponent>(TEXT("PlayerAbilityComponent"));
	TitanClimbing = CreateDefaultSubobject<UTitanClimbingComponent>(TEXT("TitanClimbing"));
	HoverGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("HoverGlow"));
	HoverGlow->SetupAttachment(GetRootComponent());
	HoverGlow->SetRelativeLocation(FVector(0.0f, 0.0f, -70.0f));
	HoverGlow->SetLightColor(FLinearColor(0.1f, 0.7f, 1.0f));
	HoverGlow->SetIntensity(1800.0f);
	HoverGlow->SetAttenuationRadius(180.0f);
	HoverGlow->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UAnimSequence> HoverAnimationAsset(
		TEXT("/Game/Resources/Soldier/Anims/Hover/AS_Soldier_Hover_M_Relaxed_Jump_Loop_Fall_UEFNRelaxed"));
	HoverAnimation = HoverAnimationAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> DashAnimationAsset(
		TEXT("/Game/Resources/References/FirstParty/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash.MM_Dash"));
	DashAnimation = DashAnimationAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbIdleAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/Source/AS_Climb_Idle.AS_Climb_Idle"));
	ClimbIdleAnimation = ClimbIdleAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbUpAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/Source/AS_Climb_Up.AS_Climb_Up"));
	ClimbUpAnimation = ClimbUpAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbDownAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/Source/AS_Climb_Down.AS_Climb_Down"));
	ClimbDownAnimation = ClimbDownAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbLeftAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/Source/AS_Climb_Left.AS_Climb_Left"));
	ClimbLeftAnimation = ClimbLeftAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbRightAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/Source/AS_Climb_Right.AS_Climb_Right"));
	ClimbRightAnimation = ClimbRightAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbMantleSequenceAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Climb_Mantling_RM_Soldier.AS_Climb_Mantling_RM_Soldier"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ClimbMantleMontageAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AM_Climb_Mantling_RM_Soldier.AM_Climb_Mantling_RM_Soldier"));

	// These are editor-created copies of Titan's original sequences.  Unlike the
	// old file-copied Soldier variants, they retain their compressed key data.
	// ReplaceSkeleton performs the required animation-data conversion as well.
	USkeleton* SoldierSkeleton = LoadObject<USkeleton>(nullptr,
		TEXT("/Game/Resources/References/FirstParty/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin"));
	if (SoldierSkeleton)
	{
		for (UAnimSequence* Animation : { ClimbIdleAnimation, ClimbUpAnimation, ClimbDownAnimation, ClimbLeftAnimation, ClimbRightAnimation })
		{
			if (Animation && Animation->GetSkeleton() != SoldierSkeleton)
			{
				Animation->ReplaceSkeleton(SoldierSkeleton, false);
			}
		}
		if (UAnimSequence* MantleSequence = ClimbMantleSequenceAsset.Object;
			MantleSequence && MantleSequence->GetSkeleton() != SoldierSkeleton)
		{
			MantleSequence->ReplaceSkeleton(SoldierSkeleton, false);
		}
		if (UAnimMontage* MantleMontage = ClimbMantleMontageAsset.Object;
			MantleMontage && MantleMontage->GetSkeleton() != SoldierSkeleton)
		{
			MantleMontage->ReplaceSkeleton(SoldierSkeleton, false);
		}
	}

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

	bUseControllerRotationYaw = false;
}

void AKzPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	TArray<UPrimitiveComponent*> PlayerPrimitiveComponents;
	GetComponents(PlayerPrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PlayerPrimitiveComponents)
	{
		if (IsValid(PrimitiveComponent))
		{
			PrimitiveComponent->SetRenderCustomDepth(true);
			PrimitiveComponent->SetCustomDepthStencilValue(3);
		}
	}

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
}

void AKzPlayerCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const bool bWasClimbUp = TitanClimbing
		&& TitanClimbing->GetClimbState() == ETitanClimbState::ClimbUp;
	if (bWasClimbUp)
	{
		bPendingClimbUpRecovery = true;
		LastClimbSurfaceNormal = TitanClimbing->GetSurfaceNormal().GetSafeNormal();
	}
	else if (bPendingClimbUpRecovery && !IsWallClimbing())
	{
		RecoverFromClimbUp();
		bPendingClimbUpRecovery = false;
	}

	UpdateClimbAnimation();
	RefreshSprintSpeed();

	if (!bHovering)
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Movement->IsMovingOnGround())
	{
		StopHover(true);
		return;
	}

	if (!Movement->IsFalling())
	{
		StopHover(false);
		return;
	}

	// Keep the character in the normal falling movement mode so collision,
	// air movement and landing continue to be handled by CharacterMovement.
	Movement->Velocity.Z = -HoverFallSpeed;
}

void AKzPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	StopHover(true);
}

AKzPlayerCharacter::~AKzPlayerCharacter() = default;

void AKzPlayerCharacter::HandleGuard() const
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleInput(EAbilityInput::Interrupt);
	}

	// TODO: 현재 능력 또는 전투 상태에 따른 방어 처리를 구현한다.
	UE_LOG(LogTemp, Log, TEXT("IA_Guard received: guard is not implemented yet."));
}

float AKzPlayerCharacter::TakeDamage(const float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount > 0.0f)
	{
		if (AbilityComponent)
		{
			AbilityComponent->HandleInput(EAbilityInput::Interrupt);
		}
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AKzPlayerCharacter::HandleAttack() const
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleInput(EAbilityInput::Interrupt);
	}
}

void AKzPlayerCharacter::SetAbility(const EAbilityType NewAbility) const
{
	if (AbilityComponent)
	{
		AbilityComponent->SetAbility(NewAbility);
	}
}

void AKzPlayerCharacter::SetPlayerState(const EPlayerState NewState)
{
	CurrentState = NewState;
}

void AKzPlayerCharacter::HandleTraversalPressed()
{
	if (TitanClimbing && TitanClimbing->IsClimbing())
	{
		TitanClimbing->Jump();
		return;
	}

	// A wall must take precedence over hover. Otherwise, pressing Space while
	// airborne near a wall starts hovering and never gives Titan a chance to grab.
	if (TitanClimbing && TitanClimbing->StartClimbing())
	{
		StopHover(false);
		return;
	}

	if (bHovering)
	{
		StopHover(false);
		return;
	}

	if (GetCharacterMovement()->IsFalling())
	{
		StartHover();
		return;
	}

	Jump();
}

void AKzPlayerCharacter::SetSprintRequested(const bool bRequested)
{
	bSprintRequested = bRequested;
	RefreshSprintSpeed();
}

void AKzPlayerCharacter::SetTraversalInput(const FVector2D& Input)
{
	ClimbInput = Input;
	if (TitanClimbing && TitanClimbing->IsClimbing())
	{
		TitanClimbing->SetClimbInput(Input);
	}
}

void AKzPlayerCharacter::UpdateClimbAnimation()
{
	const bool bIsClimbing = IsWallClimbing();
	if (!bIsClimbing)
	{
		if (bWasWallClimbing)
		{
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_Stop(0.1f);
			}
			ActiveClimbAnimation = nullptr;
		}
		bWasWallClimbing = false;
		return;
	}

	bWasWallClimbing = true;
	UAnimSequence* DesiredAnimation = ClimbIdleAnimation;
	if (ClimbInput.Y > 0.2f)
	{
		DesiredAnimation = ClimbUpAnimation;
	}
	else if (ClimbInput.Y < -0.2f)
	{
		DesiredAnimation = ClimbDownAnimation;
	}
	else if (ClimbInput.X > 0.2f)
	{
		DesiredAnimation = ClimbRightAnimation;
	}
	else if (ClimbInput.X < -0.2f)
	{
		DesiredAnimation = ClimbLeftAnimation;
	}

	PlayClimbAnimation(DesiredAnimation);
}

void AKzPlayerCharacter::PlayClimbAnimation(UAnimSequence* Animation)
{
	if (!Animation || ActiveClimbAnimation == Animation)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		// LoopCount 0 plays no iteration. Keep the selected climb clip alive until
		// the climb state or direction changes, at which point we replace it.
		AnimInstance->PlaySlotAnimationAsDynamicMontage(Animation, TEXT("DefaultSlot"), 0.1f, 0.1f, 1.0f, 999);
		ActiveClimbAnimation = Animation;
	}
}

void AKzPlayerCharacter::RecoverFromClimbUp()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Movement || !Capsule || !Movement->IsFalling() || LastClimbSurfaceNormal.IsNearlyZero())
	{
		return;
	}

	// The climb-up montage finishes in flying mode. For Soldier it may not carry
	// enough root motion to place the capsule over the ledge, so find the real
	// walkable top surface behind the climbed wall and settle the capsule on it.
	const FVector IntoLedge = -LastClimbSurfaceNormal.GetSafeNormal2D();
	if (IntoLedge.IsNearlyZero())
	{
		return;
	}

	const FVector TraceStart = GetActorLocation() + IntoLedge * 105.0f + FVector::UpVector * 180.0f;
	const FVector TraceEnd = TraceStart - FVector::UpVector * 360.0f;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SoldierClimbUpRecovery), false, this);
	FHitResult FloorHit;
	if (!GetWorld()->LineTraceSingleByChannel(FloorHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)
		|| FloorHit.ImpactNormal.Z < 0.7f)
	{
		return;
	}

	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FVector LandingLocation = FloorHit.ImpactPoint + FVector::UpVector * CapsuleHalfHeight;
	if (SetActorLocation(LandingLocation, true, nullptr, ETeleportType::TeleportPhysics))
	{
		Movement->StopMovementImmediately();
		Movement->SetMovementMode(MOVE_Walking);
	}
}

void AKzPlayerCharacter::SetDashDirection(const FVector& Direction)
{
	LastTraversalDirection = Direction.GetSafeNormal2D();
	RefreshSprintSpeed();
}

void AKzPlayerCharacter::RefreshSprintSpeed()
{
	const bool bCanSprint = bSprintRequested
		&& GetCharacterMovement()->IsMovingOnGround()
		&& !LastTraversalDirection.IsNearlyZero();
	if (bSprinting == bCanSprint)
	{
		return;
	}

	bSprinting = bCanSprint;
	GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : NormalWalkSpeed;
}

bool AKzPlayerCharacter::IsWallClimbing() const
{
	return TitanClimbing && TitanClimbing->IsClimbing();
}

bool AKzPlayerCharacter::StartHover()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (bHovering || !Movement->IsFalling())
	{
		return false;
	}

	SavedGravityScale = Movement->GravityScale;
	SavedAirControl = Movement->AirControl;
	Movement->GravityScale = 0.0f;
	Movement->AirControl = HoverAirControl;
	Movement->Velocity.Z = 0.0f;
	bHovering = true;
	HoverGlow->SetVisibility(true);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && HoverAnimation)
	{
		if (UAnimMontage* Montage = AnimInstance->PlaySlotAnimationAsDynamicMontage(HoverAnimation, TEXT("DefaultSlot"), 0.12f, 0.12f))
		{
			AnimInstance->Montage_SetNextSection(TEXT("Default"), TEXT("Default"), Montage);
		}
	}

	return true;
}

void AKzPlayerCharacter::StopHover(const bool bLanded)
{
	if (!bHovering)
	{
		return;
	}

	bHovering = false;
	HoverGlow->SetVisibility(false);
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->GravityScale = SavedGravityScale;
	Movement->AirControl = SavedAirControl;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.12f);
	}
}
