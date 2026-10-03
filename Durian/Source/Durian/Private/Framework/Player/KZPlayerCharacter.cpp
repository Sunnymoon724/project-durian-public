// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/Player/KzPlayerCharacter.h"
#include "Framework/Animation/KZPlayerAnimInstance.h"
#include "Framework/Movement/KZTitanClimbingComponent.h"

#include "Abilities/Components/AbilityEffectComponent.h"
#include "Abilities/Components/DamageableComponent.h"
#include "Abilities/Core/Ability.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "TitanClimbing.h"
#include "TitanClimbingComponent.h"

AKzPlayerCharacter::AKzPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Keep the character collision frame identical to the verified Soldier
	// playground. Titan Climbing calculates its wall anchor from this capsule;
	// the inherited Durian capsule (34 / 88) placed the Soldier mesh below the
	// expected climb frame.
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 450.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	AbilityEffect = CreateDefaultSubobject<UAbilityEffectComponent>(TEXT("AbilityEffect"));

	Damageable = CreateDefaultSubobject<UDamageableComponent>(TEXT("Damageable"));
	Damageable->SetDestroyOwnerOnDepleted(false);

	AbilityComponent = CreateDefaultSubobject<UPlayerAbilityComponent>(TEXT("PlayerAbilityComponent"));
	TitanClimbing = CreateDefaultSubobject<UKzTitanClimbingComponent>(TEXT("TitanClimbing"));
	HoverGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("HoverGlow"));
	HoverGlow->SetupAttachment(GetRootComponent());
	HoverGlow->SetRelativeLocation(FVector(0.0f, 0.0f, -70.0f));
	HoverGlow->SetLightColor(FLinearColor(0.1f, 0.7f, 1.0f));
	HoverGlow->SetIntensity(1800.0f);
	HoverGlow->SetAttenuationRadius(180.0f);
	HoverGlow->SetVisibility(false);

	static ConstructorHelpers::FClassFinder<UAnimInstance> SoldierClimbAnimBlueprint(
		TEXT("/Game/Resources/Soldier/ABP_Soldier"));
	// Keep Soldier locomotion active; climbing plays through its DefaultSlot.
	if (SoldierClimbAnimBlueprint.Class)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(SoldierClimbAnimBlueprint.Class);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbIdleAsset(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Idle"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbUpAsset(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Up"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbDownAsset(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Down"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbLeftAsset(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Left"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbRightAsset(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Right"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbJumpAsset(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Jump_Up_Soldier"));
	ClimbIdleAnimation = ClimbIdleAsset.Object;
	ClimbUpAnimation = ClimbUpAsset.Object;
	ClimbDownAnimation = ClimbDownAsset.Object;
	ClimbLeftAnimation = ClimbLeftAsset.Object;
	ClimbRightAnimation = ClimbRightAsset.Object;
	ClimbJumpAnimation = ClimbJumpAsset.Object;

	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwimIdleAsset(TEXT("/Game/Resources/Soldier/Anims/Swim/anim_SwimIdle_Soldier"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwimForwardAsset(TEXT("/Game/Resources/Soldier/Anims/Swim/anim_Swim_Surface_Fwd_Soldier"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwimLeftAsset(TEXT("/Game/Resources/Soldier/Anims/Swim/anim_Swim_Surface_Left_Soldier"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwimRightAsset(TEXT("/Game/Resources/Soldier/Anims/Swim/anim_Swim_Surface_Right_Soldier"));
	SwimIdleAnimation = SwimIdleAsset.Object;
	SwimForwardAnimation = SwimForwardAsset.Object;
	SwimLeftAnimation = SwimLeftAsset.Object;
	SwimRightAnimation = SwimRightAsset.Object;

	GetCharacterMovement()->MaxSwimSpeed = 250.0f;
	GetCharacterMovement()->BrakingDecelerationSwimming = 600.0f;
	GetCharacterMovement()->Buoyancy = 1.0f;

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;

}

void AKzPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	TitanClimbing = FindComponentByClass<UKzTitanClimbingComponent>();
	if (!TitanClimbing)
	{
		UE_LOG(LogTemp, Error, TEXT("Player is missing KzTitanClimbingComponent."));
	}

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

	// The original Soldier character receives axis input every frame, including
	// a zero value when W/A/S/D is released. Durian's enhanced-input bridge only
	// calls SetTraversalInput while an action is triggered, so clear a stale
	// climb direction here when no input arrived this frame.
	if (TitanClimbing && TitanClimbing->IsClimbing())
	{
		if (!bReceivedTraversalInputThisFrame)
		{
			ClimbInput = FVector2D::ZeroVector;
			TitanClimbing->SetClimbInput(ClimbInput);
		}
		bReceivedTraversalInputThisFrame = false;
	}
	UpdateClimbAnimation();
	UpdateSurfaceSwimming();

	RefreshSprintSpeed();
	UpdateRunStop(DeltaSeconds);

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

void AKzPlayerCharacter::HandleGuard()
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleInput(EAbilityInput::Interrupt);
	}

	if (bGuarding || IsWallClimbing() || IsWaterExitInProgress())
	{
		return;
	}

	bGuarding = true;
	if (UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->PlayGuardAnimation();
	}
}

void AKzPlayerCharacter::StopGuard()
{
	if (!bGuarding)
	{
		return;
	}

	bGuarding = false;
	if (UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->StopCombatAnimation();
	}
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

void AKzPlayerCharacter::HandleAttack()
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleInput(EAbilityInput::Interrupt);
	}

	if (IsWallClimbing() || IsWaterExitInProgress())
	{
		return;
	}

	StopGuard();
	if (UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->PlaySwordAttackAnimation();
	}

	PerformSwordHit();
}

void AKzPlayerCharacter::PlayBombThrowAnimation()
{
	if (IsWallClimbing() || IsWaterExitInProgress())
	{
		return;
	}

	if (UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->PlayBombThrowAnimation();
	}
}

void AKzPlayerCharacter::PerformSwordHit()
{
	UWorld* World = GetWorld();
	if (!World || !Controller)
	{
		return;
	}

	const FVector AttackDirection = Controller->GetControlRotation().Vector().GetSafeNormal();
	const FVector TraceStart = GetPawnViewLocation();
	const FVector TraceEnd = TraceStart + AttackDirection * 275.0f;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SwordAttackTrace), false, this);
	QueryParams.AddIgnoredActor(this);

	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return;
	}

	if (AActor* HitActor = Hit.GetActor())
	{
		UGameplayStatics::ApplyPointDamage(HitActor, 100.0f, AttackDirection, Hit, Controller, this, nullptr);
	}

	if (AbilityComponent)
	{
		AbilityComponent->HandleAttackHit(Hit, AttackDirection);
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
	CancelRunStop();
	if (IsSurfaceSwimming() || IsWaterExitInProgress())
	{
		return;
	}
	if (TitanClimbing && TitanClimbing->IsClimbing())
	{
		if (TitanClimbing->GetClimbState() == ETitanClimbState::Climb)
		{
			// Space is an upward wall leap, including while hanging still.
			SetTraversalInput(FVector2D(0.0f, 1.0f));
			TitanClimbing->Jump();
		}
		return;
	}

	// Titan's climb trace sees only actors tagged Climbable through its dedicated channel.
	UKzTitanClimbingComponent* KzClimbing = Cast<UKzTitanClimbingComponent>(TitanClimbing);
	if (KzClimbing && KzClimbing->StartClimbingOnTaggedWall())
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

void AKzPlayerCharacter::StopJumping()
{
	Super::StopJumping();
	if (TitanClimbing)
	{
		TitanClimbing->StopJumping();
	}
}

bool AKzPlayerCharacter::CancelWallClimb()
{
	if (UKzTitanClimbingComponent* Climbing = Cast<UKzTitanClimbingComponent>(TitanClimbing);
		Climbing && Climbing->IsSwimmingExitActive())
	{
		Climbing->CancelSwimmingExit();
		SetTraversalInput(FVector2D::ZeroVector);
		return true;
	}
	if (!TitanClimbing || TitanClimbing->GetClimbState() == ETitanClimbState::Idle)
	{
		return false;
	}

	StopJumping();
	SetTraversalInput(FVector2D::ZeroVector);
	TitanClimbing->StopClimbing();
	bSprintRequested = false;
	SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw, 0.0f));
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	RefreshSprintSpeed();
	return true;
}

void AKzPlayerCharacter::SetSprintRequested(const bool bRequested)
{
	bSprintRequested = bRequested;
	RefreshSprintSpeed();
}

void AKzPlayerCharacter::SetTraversalInput(const FVector2D& Input)
{
	ClimbInput = Input;
	bReceivedTraversalInputThisFrame = true;
	if (TitanClimbing && TitanClimbing->IsClimbing())
	{
		TitanClimbing->SetClimbInput(ClimbInput);
	}
}


void AKzPlayerCharacter::SetDashDirection(const FVector& Direction)
{
	if (!Direction.IsNearlyZero())
	{
		CancelRunStop();
	}
	LastTraversalDirection = Direction.GetSafeNormal2D();
	RefreshSprintSpeed();
}

void AKzPlayerCharacter::HandleMovementReleased()
{
	SetTraversalInput(FVector2D::ZeroVector);
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	const float Speed = Movement->Velocity.Size2D();
	if (!Movement->IsMovingOnGround() || IsWallClimbing() || Speed <= NormalWalkSpeed + 25.0f
		|| !AnimInstance || bRunStopOwnsBraking)
	{
		return;
	}

	// SandboxCharacter_CMC.CalculateBrakingDeceleration uses 2000 with no input.
	// Its braking friction factor is zero. CharacterMovement remains the sole
	// movement/collision authority: no launch, root-motion displacement or teleport.
	if (!AnimInstance->StartRunStop(Speed, RunStopDeceleration))
	{
		return;
	}
	SavedStopBrakingDeceleration = Movement->BrakingDecelerationWalking;
	SavedStopBrakingFrictionFactor = Movement->BrakingFrictionFactor;
	Movement->BrakingDecelerationWalking = RunStopDeceleration;
	Movement->BrakingFrictionFactor = 0.0f;
	bRunStopOwnsBraking = true;
}

void AKzPlayerCharacter::UpdateRunStop(const float DeltaSeconds)
{
	if (!bRunStopOwnsBraking)
	{
		return;
	}
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (!Movement->IsMovingOnGround() || IsWallClimbing() || !AnimInstance
		|| !AnimInstance->UpdateRunStop(Movement->Velocity.Size2D(), RunStopDeceleration, DeltaSeconds))
	{
		CancelRunStop();
		return;
	}
}

void AKzPlayerCharacter::CancelRunStop()
{
	if (!bRunStopOwnsBraking)
	{
		return;
	}
	if (UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->StopRunStop();
	}
	GetCharacterMovement()->BrakingDecelerationWalking = SavedStopBrakingDeceleration;
	GetCharacterMovement()->BrakingFrictionFactor = SavedStopBrakingFrictionFactor;
	bRunStopOwnsBraking = false;
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

bool AKzPlayerCharacter::IsSurfaceSwimming() const
{
	return GetCharacterMovement()->IsSwimming();
}

bool AKzPlayerCharacter::IsWaterExitInProgress() const
{
	const UKzTitanClimbingComponent* Climbing = Cast<UKzTitanClimbingComponent>(TitanClimbing);
	return Climbing && Climbing->IsSwimmingExitActive();
}

void AKzPlayerCharacter::UpdateSurfaceSwimming()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (Movement->IsSwimming() && CurrentState == EPlayerState::Normal
		&& ClimbInput.SizeSquared() > 0.04f
		&& FVector::DotProduct(GetActorForwardVector(), LastTraversalDirection) > 0.9f)
	{
		if (UKzTitanClimbingComponent* Climbing = Cast<UKzTitanClimbingComponent>(TitanClimbing))
		{
			const bool bSwimmingOrientation = Movement->bOrientRotationToMovement;
			Movement->bOrientRotationToMovement = bSavedOrientRotationToMovement;
			if (!Climbing->TryStartSwimmingExit())
			{
				Movement->bOrientRotationToMovement = bSwimmingOrientation;
			}
		}
	}
	const bool bSwimming = Movement->IsSwimming();

	if (bSwimming != bWasSwimming)
	{
		if (bSwimming)
		{
			StopHover(false);
			CancelRunStop();
			bSavedOrientRotationToMovement = Movement->bOrientRotationToMovement;
			Movement->bOrientRotationToMovement = false;
		}
		else if (!IsWaterExitInProgress())
		{
			Movement->bOrientRotationToMovement = bSavedOrientRotationToMovement;
		}
		bWasSwimming = bSwimming;
	}

	if (!bSwimming)
	{
		if (AnimInstance && ActiveSwimMontage)
		{
			AnimInstance->Montage_Stop(0.15f, ActiveSwimMontage);
		}
		ActiveSwimAnimation = nullptr;
		ActiveSwimMontage = nullptr;
		return;
	}

	// A PhysicsVolume marked as water supplies native swimming collision and
	// movement. Keep the capsule close to its surface; there is no dive input.
	const APhysicsVolume* WaterVolume = GetPhysicsVolume();
	if (WaterVolume && WaterVolume->bWaterVolume)
	{
		FVector Origin, Extent;
		WaterVolume->GetActorBounds(false, Origin, Extent);
		const float SurfaceZ = Origin.Z + Extent.Z;
		const float TargetZ = SurfaceZ - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.55f;
		Movement->Velocity.Z = FMath::Clamp((TargetZ - GetActorLocation().Z) * 3.0f, -100.0f, 160.0f);
	}

	if (!AnimInstance)
	{
		return;
	}

	UAnimSequence* DesiredAnimation = SwimIdleAnimation;
	if (ClimbInput.SizeSquared() > 0.04f && Movement->Velocity.SizeSquared2D() > 400.0f)
	{
		// The body now faces travel, so strafing clips would move sideways twice.
		DesiredAnimation = SwimForwardAnimation;
	}

	if (!DesiredAnimation)
	{
		return;
	}
	if (ActiveSwimAnimation != DesiredAnimation || !AnimInstance->Montage_IsPlaying(ActiveSwimMontage))
	{
		if (ActiveSwimMontage)
		{
			AnimInstance->Montage_Stop(0.12f, ActiveSwimMontage);
		}
		ActiveSwimMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
			DesiredAnimation, TEXT("DefaultSlot"), 0.12f, 0.12f);
		ActiveSwimAnimation = ActiveSwimMontage ? DesiredAnimation : nullptr;
		if (ActiveSwimMontage)
		{
			AnimInstance->Montage_SetNextSection(TEXT("Default"), TEXT("Default"), ActiveSwimMontage);
		}
	}
}

void AKzPlayerCharacter::UpdateClimbAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (!IsWallClimbing())
	{
		if (ActiveClimbMontage)
		{
			AnimInstance->Montage_Stop(0.12f, ActiveClimbMontage);
		}
		ActiveClimbAnimation = nullptr;
		ActiveClimbMontage = nullptr;
		return;
	}

	UAnimSequence* DesiredAnimation = ClimbIdleAnimation;
	if (IsWallClimbJumping() && ClimbJumpAnimation)
	{
		DesiredAnimation = ClimbJumpAnimation;
	}
	else if (ClimbInput.Y > 0.2f)
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

	if (!DesiredAnimation)
	{
		return;
	}
	if (ActiveClimbAnimation != DesiredAnimation || !AnimInstance->Montage_IsPlaying(ActiveClimbMontage))
	{
		if (ActiveClimbMontage)
		{
			AnimInstance->Montage_Stop(0.08f, ActiveClimbMontage);
		}
		ActiveClimbMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(DesiredAnimation, TEXT("DefaultSlot"), 0.08f, 0.08f);
		ActiveClimbAnimation = ActiveClimbMontage ? DesiredAnimation : nullptr;
		if (ActiveClimbMontage && !IsWallClimbJumping())
		{
			AnimInstance->Montage_SetNextSection(TEXT("Default"), TEXT("Default"), ActiveClimbMontage);
		}
	}

	if (IsWallClimbJumping() && ActiveClimbMontage)
	{
		AnimInstance->Montage_SetPlayRate(ActiveClimbMontage, 0.0f);
		AnimInstance->Montage_SetPosition(ActiveClimbMontage, FMath::Clamp(GetWallClimbJumpProgress(), 0.0f, 1.0f) * DesiredAnimation->GetPlayLength());
	}
}

bool AKzPlayerCharacter::IsWallClimbJumping() const
{
	return TitanClimbing && TitanClimbing->GetClimbState() == ETitanClimbState::Jump;
}

float AKzPlayerCharacter::GetWallClimbJumpProgress() const
{
	return TitanClimbing ? TitanClimbing->GetAnimData().JumpProgress : 0.0f;
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

	if (UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->PlayHoverAnimation();
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

	if (UKZPlayerAnimInstance* AnimInstance = Cast<UKZPlayerAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInstance->StopHoverAnimation();
	}
}
