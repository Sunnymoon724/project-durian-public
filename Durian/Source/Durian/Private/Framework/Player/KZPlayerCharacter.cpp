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
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TitanClimbing.h"
#include "TitanClimbingComponent.h"
#include "Framework/Player/WallClimbComponent.h"

AKzPlayerCharacter::AKzPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilityEffect = CreateDefaultSubobject<UAbilityEffectComponent>(TEXT("AbilityEffect"));

	Damageable = CreateDefaultSubobject<UDamageableComponent>(TEXT("Damageable"));
	Damageable->SetDestroyOwnerOnDepleted(false);

	AbilityComponent = CreateDefaultSubobject<UPlayerAbilityComponent>(TEXT("PlayerAbilityComponent"));
	WallClimb = CreateDefaultSubobject<UWallClimbComponent>(TEXT("WallClimb"));
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
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Idle.AS_Soldier_Titan_AS_Climb_Idle"));
	ClimbIdleAnimation = ClimbIdleAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbUpAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Up.AS_Soldier_Titan_AS_Climb_Up"));
	ClimbUpAnimation = ClimbUpAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbDownAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Down.AS_Soldier_Titan_AS_Climb_Down"));
	ClimbDownAnimation = ClimbDownAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbLeftAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Left.AS_Soldier_Titan_AS_Climb_Left"));
	ClimbLeftAnimation = ClimbLeftAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbRightAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Right.AS_Soldier_Titan_AS_Climb_Right"));
	ClimbRightAnimation = ClimbRightAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbMantleSequenceAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Climb_Mantling_RM_Soldier.AS_Climb_Mantling_RM_Soldier"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ClimbMantleMontageAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AM_Climb_Mantling_RM_Soldier.AM_Climb_Mantling_RM_Soldier"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwordAttack0Asset(
		TEXT("/Game/Resources/Soldier/Anims/SwordAndShield/Combo_Attack_01_01_Seq.Combo_Attack_01_01_Seq"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwordBlockAsset(
		TEXT("/Game/Resources/Soldier/Anims/SwordAndShield/Block_Loop_Seq.Block_Loop_Seq"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> BombThrowAsset(
		TEXT("/Game/Resources/Soldier/Anims/Abilities/Bomb/AS_Soldier_Bomb_AN_ANIM_ThrowING_Stealth.AS_Soldier_Bomb_AN_ANIM_ThrowING_Stealth"));
	SwordAttack0Animation = SwordAttack0Asset.Object;
	SwordBlockAnimation = SwordBlockAsset.Object;
	BombThrowAnimation = BombThrowAsset.Object;
	static ConstructorHelpers::FClassFinder<UAnimInstance> SoldierClimbAnimBlueprint(
		TEXT("/Game/Resources/Soldier/ABP_Soldier"));
	// Soldier locomotion and Titan climbing share this animation blueprint.
	// Keep it active for the full lifetime of the player; do not swap graphs
	// when a climb begins, because that creates conflicting animation state.
	if (SoldierClimbAnimBlueprint.Class)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(SoldierClimbAnimBlueprint.Class);
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

void AKzPlayerCharacter::HandleGuard()
{
	if (AbilityComponent)
	{
		AbilityComponent->HandleInput(EAbilityInput::Interrupt);
	}

	if (bGuarding || !SwordBlockAnimation || IsWallClimbing())
	{
		return;
	}

	bGuarding = true;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		ActiveCombatMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
			SwordBlockAnimation, TEXT("DefaultSlot"), 0.1f, 0.1f);
		if (ActiveCombatMontage)
		{
			AnimInstance->Montage_SetNextSection(TEXT("Default"), TEXT("Default"), ActiveCombatMontage);
		}
	}
}

void AKzPlayerCharacter::StopGuard()
{
	if (!bGuarding)
	{
		return;
	}

	bGuarding = false;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.1f, ActiveCombatMontage);
	}
	ActiveCombatMontage = nullptr;
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

	if (IsWallClimbing())
	{
		return;
	}

	StopGuard();
	if (SwordAttack0Animation)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			ActiveCombatMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
				SwordAttack0Animation, TEXT("DefaultSlot"), 0.08f, 0.08f);
		}
	}

	PerformSwordHit();
}

void AKzPlayerCharacter::PlayBombThrowAnimation()
{
	if (IsWallClimbing() || !BombThrowAnimation)
	{
		return;
	}

	StopGuard();
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		ActiveCombatMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
			BombThrowAnimation, TEXT("DefaultSlot"), 0.08f, 0.08f);
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
	bReceivedTraversalInputThisFrame = true;
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
				AnimInstance->Montage_Stop(0.12f, ActiveClimbMontage);
			}
			ActiveClimbMontage = nullptr;
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
		AnimInstance->Montage_Stop(0.08f, ActiveClimbMontage);
		ActiveClimbMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
			Animation, TEXT("DefaultSlot"), 0.08f, 0.08f);
		if (ActiveClimbMontage)
		{
			AnimInstance->Montage_SetNextSection(TEXT("Default"), TEXT("Default"), ActiveClimbMontage);
			ActiveClimbAnimation = Animation;
		}
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
