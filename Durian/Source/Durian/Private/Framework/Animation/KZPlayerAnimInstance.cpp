#include "Framework/Animation/KZPlayerAnimInstance.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UKZPlayerAnimInstance::UKZPlayerAnimInstance()
{
	static ConstructorHelpers::FObjectFinder<UAnimSequence> HoverAsset(
		TEXT("/Game/Resources/Soldier/Anims/Hover/AS_Soldier_Hover_M_Relaxed_Jump_Loop_Fall_UEFNRelaxed"));
	HoverAnimation = HoverAsset.Object;
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
	static ConstructorHelpers::FObjectFinder<UAnimSequence> ClimbJumpAsset(
		TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Jump_Up_Soldier.AS_Jump_Up_Soldier"));
	ClimbJumpAnimation = ClimbJumpAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwordAttackAsset(
		TEXT("/Game/Resources/Soldier/Anims/SwordAndShield/Combo_Attack_01_01_Seq.Combo_Attack_01_01_Seq"));
	SwordAttack0Animation = SwordAttackAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> SwordBlockAsset(
		TEXT("/Game/Resources/Soldier/Anims/SwordAndShield/Block_Loop_Seq.Block_Loop_Seq"));
	SwordBlockAnimation = SwordBlockAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> BombThrowAsset(
		TEXT("/Game/Resources/Soldier/Anims/Abilities/Bomb/AS_Soldier_Bomb_AN_ANIM_ThrowING_Stealth.AS_Soldier_Bomb_AN_ANIM_ThrowING_Stealth"));
	BombThrowAnimation = BombThrowAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> RunStopLeftAsset(
		TEXT("/Game/Resources/Soldier/Anims/GameAnimationSample/DistanceMatchedStop/AS_Soldier_GASP_Stop_L"));
	RunStopLeftAnimation = RunStopLeftAsset.Object;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> RunStopRightAsset(
		TEXT("/Game/Resources/Soldier/Anims/GameAnimationSample/DistanceMatchedStop/AS_Soldier_GASP_Stop_R"));
	RunStopRightAnimation = RunStopRightAsset.Object;
}

void UKZPlayerAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AKzPlayerCharacter* Player = Cast<AKzPlayerCharacter>(TryGetPawnOwner());
	if (!Player)
	{
		Speed = 0.0f;
		bIsInAir = false;
		bIsWallClimbing = false;
		bIsHovering = false;
		ClimbForwardInput = 0.0f;
		return;
	}

	const UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	Speed = Movement ? Movement->Velocity.Size2D() : 0.0f;
	bIsInAir = Movement && Movement->IsFalling();
	bIsWallClimbing = Player->IsWallClimbing();
	bIsHovering = Player->IsHovering();
	ClimbForwardInput = Player->GetForwardInputValue();
	UpdateClimbAnimation(*Player);
}

void UKZPlayerAnimInstance::PlayHoverAnimation()
{
	if (!HoverAnimation)
	{
		return;
	}
	if (ActiveHoverMontage && Montage_IsPlaying(ActiveHoverMontage))
	{
		return;
	}
	ActiveHoverMontage = nullptr;
	ActiveHoverMontage = PlaySlotAnimationAsDynamicMontage(HoverAnimation, TEXT("DefaultSlot"), 0.12f, 0.12f);
	if (ActiveHoverMontage)
	{
		Montage_SetNextSection(TEXT("Default"), TEXT("Default"), ActiveHoverMontage);
	}
}

void UKZPlayerAnimInstance::StopHoverAnimation()
{
	if (ActiveHoverMontage)
	{
		Montage_Stop(0.12f, ActiveHoverMontage);
		ActiveHoverMontage = nullptr;
	}
}

void UKZPlayerAnimInstance::PlayGuardAnimation()
{
	if (!SwordBlockAnimation)
	{
		return;
	}
	ActiveCombatMontage = PlaySlotAnimationAsDynamicMontage(SwordBlockAnimation, TEXT("DefaultSlot"), 0.1f, 0.1f);
	if (ActiveCombatMontage)
	{
		Montage_SetNextSection(TEXT("Default"), TEXT("Default"), ActiveCombatMontage);
	}
}

void UKZPlayerAnimInstance::StopCombatAnimation()
{
	if (ActiveCombatMontage)
	{
		Montage_Stop(0.1f, ActiveCombatMontage);
		ActiveCombatMontage = nullptr;
	}
}

void UKZPlayerAnimInstance::PlaySwordAttackAnimation()
{
	if (SwordAttack0Animation)
	{
		ActiveCombatMontage = PlaySlotAnimationAsDynamicMontage(SwordAttack0Animation, TEXT("DefaultSlot"), 0.08f, 0.08f);
	}
}

void UKZPlayerAnimInstance::PlayBombThrowAnimation()
{
	if (BombThrowAnimation)
	{
		ActiveCombatMontage = PlaySlotAnimationAsDynamicMontage(BombThrowAnimation, TEXT("DefaultSlot"), 0.08f, 0.08f);
	}
}

float UKZPlayerAnimInstance::FindRunStopTime(const UAnimSequence* Animation, const float RemainingDistance) const
{
	float Low = 0.0f;
	float High = 1.0f;
	for (int32 Iteration = 0; Iteration < 16; ++Iteration)
	{
		const float Mid = (Low + High) * 0.5f;
		const float Distance = Animation->EvaluateCurveData(TEXT("StopDistance"), FAnimExtractContext(Mid, false));
		if (Distance > RemainingDistance)
		{
			Low = Mid;
		}
		else
		{
			High = Mid;
		}
	}
	return (Low + High) * 0.5f;
}

float UKZPlayerAnimInstance::GetRunStopPoseCost(const UAnimSequence* Animation, const float Time) const
{
	const FReferenceSkeleton& Reference = Animation->GetSkeleton()->GetReferenceSkeleton();
	float Cost = 0.0f;
	for (const FName Bone : { FName(TEXT("foot_l")), FName(TEXT("foot_r")) })
	{
		FTransform ComponentPose = FTransform::Identity;
		for (int32 Index = Reference.FindBoneIndex(Bone); Index != INDEX_NONE; Index = Reference.GetParentIndex(Index))
		{
			FTransform LocalPose;
			Animation->GetBoneTransform(LocalPose, FSkeletonPoseBoneIndex(Index), FAnimExtractContext(Time, false), false);
			ComponentPose *= LocalPose;
		}
		const FVector Current = GetSkelMeshComponent()->GetSocketTransform(Bone, RTS_Component).GetLocation();
		Cost += FVector::DistSquared(Current, ComponentPose.GetLocation());
	}
	return Cost;
}

bool UKZPlayerAnimInstance::StartRunStop(const float CurrentSpeed, const float Deceleration)
{
	if (IsAnyMontagePlaying() || !RunStopLeftAnimation || !RunStopRightAnimation || !GetSkelMeshComponent())
	{
		return false;
	}
	const float Distance = CurrentSpeed * CurrentSpeed / (2.0f * Deceleration);
	const float LeftTime = FindRunStopTime(RunStopLeftAnimation, Distance);
	const float RightTime = FindRunStopTime(RunStopRightAnimation, Distance);
	const bool bUseLeft = GetRunStopPoseCost(RunStopLeftAnimation, LeftTime)
		<= GetRunStopPoseCost(RunStopRightAnimation, RightTime);
	ActiveRunStopAnimation = bUseLeft ? RunStopLeftAnimation : RunStopRightAnimation;
	RunStopTime = bUseLeft ? LeftTime : RightTime;
	ActiveRunStopMontage = PlaySlotAnimationAsDynamicMontage(
		ActiveRunStopAnimation, TEXT("DefaultSlot"), 0.07f, 0.15f, 1.0f, 1, -1.0f, RunStopTime);
	if (!ActiveRunStopMontage)
	{
		ActiveRunStopAnimation = nullptr;
		return false;
	}
	Montage_SetPlayRate(ActiveRunStopMontage, 0.0f);
	return true;
}

bool UKZPlayerAnimInstance::UpdateRunStop(const float CurrentSpeed, const float Deceleration, const float DeltaSeconds)
{
	if (!ActiveRunStopAnimation || GetCurrentActiveMontage() != ActiveRunStopMontage)
	{
		return false;
	}
	if (CurrentSpeed > 1.0f)
	{
		RunStopTime = FMath::Max(RunStopTime,
			FindRunStopTime(ActiveRunStopAnimation, CurrentSpeed * CurrentSpeed / (2.0f * Deceleration)));
	}
	else
	{
		RunStopTime = FMath::Max(RunStopTime, 1.0f) + DeltaSeconds;
	}
	Montage_SetPosition(ActiveRunStopMontage, RunStopTime);
	return RunStopTime < 1.5f;
}

void UKZPlayerAnimInstance::StopRunStop(const float BlendOutTime)
{
	if (ActiveRunStopMontage)
	{
		Montage_Stop(BlendOutTime, ActiveRunStopMontage);
	}
	ActiveRunStopMontage = nullptr;
	ActiveRunStopAnimation = nullptr;
	RunStopTime = 0.0f;
}

void UKZPlayerAnimInstance::UpdateClimbAnimation(const AKzPlayerCharacter& Player)
{
	if (!Player.IsWallClimbing())
	{
		if (bWasWallClimbing && ActiveClimbMontage)
		{
			Montage_Stop(0.12f, ActiveClimbMontage);
		}
		bWasWallClimbing = false;
		ActiveClimbMontage = nullptr;
		ActiveClimbAnimation = nullptr;
		return;
	}

	bWasWallClimbing = true;
	if (Player.IsWallClimbJumping() && ClimbJumpAnimation)
	{
		PlayClimbAnimation(ClimbJumpAnimation, false);
		if (ActiveClimbMontage)
		{
			Montage_SetPlayRate(ActiveClimbMontage, 0.0f);
			Montage_SetPosition(ActiveClimbMontage,
				FMath::Clamp(Player.GetWallClimbJumpProgress(), 0.0f, 1.0f) * ClimbJumpAnimation->GetPlayLength());
		}
		return;
	}

	const FVector2D Input = Player.GetClimbInput();
	UAnimSequence* DesiredAnimation = ClimbIdleAnimation;
	if (Input.Y > 0.2f) DesiredAnimation = ClimbUpAnimation;
	else if (Input.Y < -0.2f) DesiredAnimation = ClimbDownAnimation;
	else if (Input.X > 0.2f) DesiredAnimation = ClimbRightAnimation;
	else if (Input.X < -0.2f) DesiredAnimation = ClimbLeftAnimation;
	PlayClimbAnimation(DesiredAnimation);
}

void UKZPlayerAnimInstance::PlayClimbAnimation(UAnimSequence* Animation, const bool bLoop)
{
	if (!Animation || ActiveClimbAnimation == Animation)
	{
		return;
	}
	Montage_Stop(0.08f, ActiveClimbMontage);
	ActiveClimbMontage = PlaySlotAnimationAsDynamicMontage(Animation, TEXT("DefaultSlot"), 0.08f, 0.08f);
	if (ActiveClimbMontage)
	{
		if (bLoop)
		{
			Montage_SetNextSection(TEXT("Default"), TEXT("Default"), ActiveClimbMontage);
		}
		ActiveClimbAnimation = Animation;
	}
}
