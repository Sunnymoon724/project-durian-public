#include "Framework/Player/KzSoldierAnimationAssetUtility.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Animation/Skeleton.h"
#include "UObject/SoftObjectPath.h"

int32 UKzSoldierAnimationAssetUtility::RepairTraversalAnimationSkeletons()
{
	USkeleton* TargetSkeleton = LoadObject<USkeleton>(nullptr,
		TEXT("/Game/Resources/References/FirstParty/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin"));
	if (!TargetSkeleton)
	{
		return 0;
	}

	const TArray<FSoftObjectPath> AnimationPaths = {
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/BS_Idle_Walk_Run.BS_Idle_Walk_Run")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/MM_Idle.MM_Idle")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Walk/MF_Unarmed_Walk_Bwd_Left.MF_Unarmed_Walk_Bwd_Left")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Walk/MF_Unarmed_Walk_Bwd_Right.MF_Unarmed_Walk_Bwd_Right")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Walk/MF_Unarmed_Walk_Bwd.MF_Unarmed_Walk_Bwd")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd_Left.MF_Unarmed_Walk_Fwd_Left")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd_Right.MF_Unarmed_Walk_Fwd_Right")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd.MF_Unarmed_Walk_Fwd")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Walk/MF_Unarmed_Walk_Left.MF_Unarmed_Walk_Left")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Walk/MF_Unarmed_Walk_Right.MF_Unarmed_Walk_Right")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jog/MF_Unarmed_Jog_Bwd_Left.MF_Unarmed_Jog_Bwd_Left")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jog/MF_Unarmed_Jog_Bwd_Right.MF_Unarmed_Jog_Bwd_Right")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jog/MF_Unarmed_Jog_Bwd.MF_Unarmed_Jog_Bwd")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd_Left.MF_Unarmed_Jog_Fwd_Left")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd_Right.MF_Unarmed_Jog_Fwd_Right")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd.MF_Unarmed_Jog_Fwd")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jog/MF_Unarmed_Jog_Left.MF_Unarmed_Jog_Left")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jog/MF_Unarmed_Jog_Right.MF_Unarmed_Jog_Right")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Idle.AS_Soldier_Titan_AS_Climb_Idle")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Up.AS_Soldier_Titan_AS_Climb_Up")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Down.AS_Soldier_Titan_AS_Climb_Down")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Left.AS_Soldier_Titan_AS_Climb_Left")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Traversal/Titan/AS_Soldier_Titan_AS_Climb_Right.AS_Soldier_Titan_AS_Climb_Right")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/SwordAndShield/Block_Loop_Seq.Block_Loop_Seq")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/SwordAndShield/Combo_Attack_01_All_Seq.Combo_Attack_01_All_Seq")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/SwordAndShield/Combo_Attack_02_All_Seq.Combo_Attack_02_All_Seq")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/SwordAndShield/Idle_Combat_Seq.Idle_Combat_Seq")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Unarmed/Jump/MM_Dash.MM_Dash")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Hover/AS_Soldier_Hover_M_Relaxed_Jump_Loop_Fall_UEFNRelaxed.AS_Soldier_Hover_M_Relaxed_Jump_Loop_Fall_UEFNRelaxed")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Traversal/M_Neutral_Traversal_Catch_Mantle_med_stand_SoldierFixed.M_Neutral_Traversal_Catch_Mantle_med_stand_SoldierFixed")),
		FSoftObjectPath(TEXT("/Game/Resources/Soldier/Anims/Traversal/M_Neutral_Traversal_Climb_Start_2_5_stand_F_Lfoot_SoldierFixed.M_Neutral_Traversal_Climb_Start_2_5_stand_F_Lfoot_SoldierFixed"))
	};

	int32 RepairedCount = 0;
	for (const FSoftObjectPath& AnimationPath : AnimationPaths)
	{
		if (UAnimationAsset* Animation = Cast<UAnimationAsset>(AnimationPath.TryLoad()))
		{
			if (Animation->GetSkeleton() != TargetSkeleton)
			{
				Animation->SetSkeleton(TargetSkeleton);
				Animation->MarkPackageDirty();
				++RepairedCount;
			}
		}
	}

	if (UAnimBlueprint* SoldierABP = LoadObject<UAnimBlueprint>(nullptr,
		TEXT("/Game/Resources/Soldier/ABP_Soldier.ABP_Soldier")))
	{
		if (SoldierABP->TargetSkeleton != TargetSkeleton || !SoldierABP->ParentClass)
		{
			SoldierABP->TargetSkeleton = TargetSkeleton;
			SoldierABP->ParentClass = UAnimInstance::StaticClass();
			SoldierABP->MarkPackageDirty();
			++RepairedCount;
		}
	}

	return RepairedCount;
}
