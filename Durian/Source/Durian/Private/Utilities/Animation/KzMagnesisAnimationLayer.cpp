#include "Utilities/Animation/KzSoldierAnimationAssetUtility.h"
#include "Animation/AnimInstance.h"
#include "Framework/Player/KzPlayerCharacter.h"

#if WITH_EDITOR
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "UObject/Package.h"
#include "Animation/Skeleton.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_SaveCachedPose.h"
#include "AnimGraphNode_UseCachedPose.h"
#include "AnimGraphNode_Slot.h"
#include "AnimGraphNode_LayeredBoneBlend.h"
#include "AnimGraphNode_BlendSpacePlayer.h"
#include "AnimGraphNode_BlendListByBool.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Self.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "EdGraphSchema_K2.h"
#include "Animation/BlendSpace.h"
#include "Animation/MirrorDataTable.h"
#include "Animation/AnimationSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EdGraph/EdGraphSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/CompilerResultsLog.h"
#endif

#if WITH_EDITOR
namespace
{
const FName MagnesisStateVariable(TEXT("MagnesisStrafing"));

bool CacheMagnesisStateOnGameThread(UAnimBlueprint* BP, const TArray<UEdGraph*>& Graphs)
{
	// Read the pawn during BlueprintUpdateAnimation, not from a worker-thread
	// exposed input in the pose graph. AnimGraph only reads the cached boolean.
	if (FBlueprintEditorUtils::FindNewVariableIndex(BP, MagnesisStateVariable) == INDEX_NONE)
	{
		FEdGraphPinType Type;
		Type.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		if (!FBlueprintEditorUtils::AddMemberVariable(BP, MagnesisStateVariable, Type)) return false;
	}
	for (UEdGraph* Graph : Graphs)
	{
		UK2Node_Event* Event = nullptr;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (auto* Set = Cast<UK2Node_VariableSet>(Node);
				Set && Set->VariableReference.GetMemberName() == MagnesisStateVariable) return true;
			if (auto* Candidate = Cast<UK2Node_Event>(Node);
				Candidate && Candidate->EventReference.GetMemberName() == TEXT("BlueprintUpdateAnimation")) Event = Candidate;
		}
		if (!Event) continue;
		UEdGraphPin* Exec = Event->FindPin(UEdGraphSchema_K2::PN_Then);
		if (!Exec || Exec->LinkedTo.Num() != 1) return false;
		UEdGraphPin* Next = Exec->LinkedTo[0];
		FGraphNodeCreator<UK2Node_VariableSet> SetCreator(*Graph);
		auto* Set = SetCreator.CreateNode(false);
		Set->VariableReference.SetSelfMember(MagnesisStateVariable);
		Set->NodePosX = Event->NodePosX + 220;
		Set->NodePosY = Event->NodePosY - 180;
		SetCreator.Finalize();
		FGraphNodeCreator<UK2Node_CallFunction> CallCreator(*Graph);
		auto* Call = CallCreator.CreateNode(false);
		Call->SetFromFunction(UKzSoldierAnimationAssetUtility::StaticClass()->FindFunctionByName(TEXT("IsMagnesisHolding")));
		Call->NodePosX = Event->NodePosX - 180;
		Call->NodePosY = Event->NodePosY - 360;
		CallCreator.Finalize();
		FGraphNodeCreator<UK2Node_Self> SelfCreator(*Graph);
		auto* Self = SelfCreator.CreateNode(false);
		Self->NodePosX = Call->NodePosX - 180;
		Self->NodePosY = Call->NodePosY;
		SelfCreator.Finalize();
		const UEdGraphSchema* Schema = Graph->GetSchema();
		Exec->BreakAllPinLinks();
		return Schema->TryCreateConnection(Self->FindPin(TEXT("self")), Call->FindPin(TEXT("Animation")))
			&& Schema->TryCreateConnection(Call->FindPin(TEXT("ReturnValue")), Set->FindPin(MagnesisStateVariable))
			&& Schema->TryCreateConnection(Exec, Set->FindPin(UEdGraphSchema_K2::PN_Execute))
			&& Schema->TryCreateConnection(Set->FindPin(UEdGraphSchema_K2::PN_Then), Next);
	}
	return false;
}
}
#endif

bool UKzSoldierAnimationAssetUtility::IsMagnesisHolding(const UAnimInstance* Animation)
{
	const AKzPlayerCharacter* Player = Animation ? Cast<AKzPlayerCharacter>(Animation->GetOwningActor()) : nullptr;
	return Player && Player->GetCurrentState() == EPlayerState::MagnesisHolding;
}

bool UKzSoldierAnimationAssetUtility::InstallMagnesisStrafeBlend()
{
#if WITH_EDITOR
	UAnimBlueprint* BP = LoadObject<UAnimBlueprint>(nullptr, TEXT("/Game/Resources/Soldier/ABP_Soldier.ABP_Soldier"));
	UBlendSpace* Strafe = LoadObject<UBlendSpace>(nullptr, TEXT("/Game/Resources/Soldier/Anims/Abilities/Magnesis/BS_Soldier_MagnesisMove.BS_Soldier_MagnesisMove"));
	if (!BP || !Strafe) return false;
	const TCHAR* MirrorPackage = TEXT("/Game/Resources/Soldier/Anims/Abilities/Magnesis/MDT_Soldier_Magnesis");
	UMirrorDataTable* Mirror = LoadObject<UMirrorDataTable>(nullptr,
		TEXT("/Game/Resources/Soldier/Anims/Abilities/Magnesis/MDT_Soldier_Magnesis.MDT_Soldier_Magnesis"));
	if (!Mirror)
	{
		Mirror = NewObject<UMirrorDataTable>(CreatePackage(MirrorPackage), TEXT("MDT_Soldier_Magnesis"), RF_Public | RF_Standalone);
		Mirror->RowStruct = FMirrorTableRow::StaticStruct();
		Mirror->Skeleton = Strafe->GetSkeleton();
		Mirror->MirrorAxis = EAxis::X;
		Mirror->MirrorFindReplaceExpressions = UAnimationSettings::Get()->MirrorFindReplaceExpressions;
		Mirror->UpdateFromFindReplaceExpressions(UMirrorDataTable::FFindReplaceOptions::Sync());
		FAssetRegistryModule::AssetCreated(Mirror);
		Mirror->MarkPackageDirty();
	}
	Strafe->MirrorDataTable = Mirror;
	Strafe->MarkPackageDirty();
	TArray<UEdGraph*> Graphs;
	BP->GetAllGraphs(Graphs);
	if (!CacheMagnesisStateOnGameThread(BP, Graphs)) return false;
	for (UEdGraph* Graph : Graphs)
	{
		if (Graph->GetName() != TEXT("Walk / Run")) continue;
		UAnimGraphNode_BlendSpacePlayer* Original = nullptr;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (auto* Player = Cast<UAnimGraphNode_BlendSpacePlayer>(Node))
			{
				if (Player->Node.GetBlendSpace() == Strafe)
				{
					// Upgrade an already installed layer to cached, thread-safe inputs.
					TArray<UK2Node_CallFunction*> OldCalls;
					for (UEdGraphNode* N : Graph->Nodes)
					{
						if (auto* C = Cast<UK2Node_CallFunction>(N);
							C && C->FunctionReference.GetMemberName() == TEXT("IsMagnesisHolding")) OldCalls.Add(C);
					}
					for (UK2Node_CallFunction* C : OldCalls)
					{
						FGraphNodeCreator<UK2Node_VariableGet> GetCreator(*Graph);
						auto* Get = GetCreator.CreateNode(false);
						Get->VariableReference.SetSelfMember(MagnesisStateVariable);
						Get->NodePosX = C->NodePosX;
						Get->NodePosY = C->NodePosY;
						GetCreator.Finalize();
						const auto Links = C->FindPin(TEXT("ReturnValue"))->LinkedTo;
						for (UEdGraphPin* Pin : Links)
						{
							Pin->BreakLinkTo(C->FindPin(TEXT("ReturnValue")));
							if (!Graph->GetSchema()->TryCreateConnection(Get->FindPin(MagnesisStateVariable), Pin)) return false;
						}
						FBlueprintEditorUtils::RemoveNode(BP, C, true);
					}
					FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
					FCompilerResultsLog Log;
					FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::None, &Log);
					return Log.NumErrors == 0 && BP->Status != BS_Error;
				}
				Original = Player;
			}
		}
		UEdGraphPin* OriginalOut = Original ? Original->FindPin(TEXT("Pose")) : nullptr;
		if (!OriginalOut || OriginalOut->LinkedTo.Num() != 1) return false;
		UEdGraphPin* Result = OriginalOut->LinkedTo[0];
		BP->Modify();
		Graph->Modify();
		TArray<UEdGraphNode*> Added;
		FGraphNodeCreator<UAnimGraphNode_BlendSpacePlayer> PlayerCreator(*Graph);
		auto* Player = PlayerCreator.CreateNode(false);
		Player->Node = Original->Node;
		Player->Node.SetBlendSpace(Strafe);
		Player->NodePosX = Original->NodePosX;
		Player->NodePosY = Original->NodePosY + 260;
		PlayerCreator.Finalize();
		Added.Add(Player);
		FGraphNodeCreator<UAnimGraphNode_BlendListByBool> BlendCreator(*Graph);
		auto* Blend = BlendCreator.CreateNode(false);
		Blend->NodePosX = Original->NodePosX + 360;
		Blend->NodePosY = Original->NodePosY;
		BlendCreator.Finalize();
		Added.Add(Blend);
		FGraphNodeCreator<UK2Node_VariableGet> GetCreator(*Graph);
		auto* Get = GetCreator.CreateNode(false);
		Get->VariableReference.SetSelfMember(MagnesisStateVariable);
		Get->NodePosX = Original->NodePosX;
		Get->NodePosY = Original->NodePosY - 210;
		GetCreator.Finalize();
		Added.Add(Get);
		const UEdGraphSchema* Schema = Graph->GetSchema();
		auto Link = [Schema](UEdGraphPin* A, UEdGraphPin* B) { return A && B && Schema->TryCreateConnection(A, B); };
		bool bInputs = true;
		for (UEdGraphPin* Pin : Original->Pins)
		{
			if (Pin->Direction != EGPD_Input) continue;
			if (UEdGraphPin* Copy = Player->FindPin(Pin->PinName))
			{
				Copy->DefaultValue = Pin->DefaultValue;
				for (UEdGraphPin* Source : Pin->LinkedTo) bInputs &= Link(Source, Copy);
			}
		}
		Result->BreakAllPinLinks();
		const bool bLinked = bInputs && Link(Get->FindPin(MagnesisStateVariable), Blend->FindPin(TEXT("bActiveValue")))
			&& Link(Player->FindPin(TEXT("Pose")), Blend->FindPin(TEXT("BlendPose_0")))
			&& Link(OriginalOut, Blend->FindPin(TEXT("BlendPose_1")))
			&& Link(Blend->FindPin(TEXT("Pose")), Result);
		if (bLinked)
		{
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
			FCompilerResultsLog Log;
			FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::None, &Log);
			if (Log.NumErrors == 0 && BP->Status != BS_Error) return true;
		}
		for (UEdGraphNode* Node : Added) FBlueprintEditorUtils::RemoveNode(BP, Node, true);
		Result->BreakAllPinLinks();
		OriginalOut->MakeLinkTo(Result);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
		FKismetEditorUtilities::CompileBlueprint(BP);
		return false;
	}
#endif
	return false;
}

bool UKzSoldierAnimationAssetUtility::InstallMagnesisUpperBodyLayer()
{
#if WITH_EDITOR
	UAnimBlueprint* Blueprint = LoadObject<UAnimBlueprint>(nullptr, TEXT("/Game/Resources/Soldier/ABP_Soldier.ABP_Soldier"));
	if (!Blueprint || !Blueprint->TargetSkeleton)
	{
		return false;
	}
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	UEdGraph* Graph = nullptr;
	UAnimGraphNode_Root* Root = nullptr;
	for (UEdGraph* Candidate : Graphs)
	{
		if (Candidate->GetFName() != TEXT("AnimGraph")) continue;
		Graph = Candidate;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (const UAnimGraphNode_Slot* Slot = Cast<UAnimGraphNode_Slot>(Node);
				Slot && Slot->Node.SlotName == TEXT("MagnesisUpperBody"))
			{
				return Blueprint->Status != BS_Error;
			}
			if (UAnimGraphNode_Root* Found = Cast<UAnimGraphNode_Root>(Node)) Root = Found;
		}
		break;
	}
	UEdGraphPin* Result = Root ? Root->FindPin(TEXT("Result")) : nullptr;
	if (!Graph || !Result || Result->LinkedTo.Num() != 1) return false;
	UEdGraphPin* Original = Result->LinkedTo[0];
	Blueprint->Modify();
	Graph->Modify();
	TArray<UEdGraphNode*> Added;

	FGraphNodeCreator<UAnimGraphNode_SaveCachedPose> SaveCreator(*Graph);
	auto* Save = SaveCreator.CreateNode(false);
	Save->CacheName = TEXT("MagnesisBasePose");
	Save->NodePosX = Root->NodePosX - 900;
	Save->NodePosY = Root->NodePosY + 320;
	SaveCreator.Finalize();
	Added.Add(Save);
	auto MakeUse = [&](int32 Y)
	{
		FGraphNodeCreator<UAnimGraphNode_UseCachedPose> Creator(*Graph);
		auto* Use = Creator.CreateNode(false);
		Use->SaveCachedPoseNode = Save;
		Use->NodePosX = Root->NodePosX - 900;
		Use->NodePosY = Root->NodePosY + Y;
		Creator.Finalize();
		Added.Add(Use);
		return Use;
	};
	auto* Base = MakeUse(0);
	auto* Upper = MakeUse(160);
	FGraphNodeCreator<UAnimGraphNode_Slot> SlotCreator(*Graph);
	auto* Slot = SlotCreator.CreateNode(false);
	Slot->Node.SlotName = TEXT("MagnesisUpperBody");
	Slot->Node.bAlwaysUpdateSourcePose = true;
	Slot->NodePosX = Root->NodePosX - 650;
	Slot->NodePosY = Root->NodePosY + 160;
	SlotCreator.Finalize();
	Added.Add(Slot);
	FGraphNodeCreator<UAnimGraphNode_LayeredBoneBlend> BlendCreator(*Graph);
	auto* Blend = BlendCreator.CreateNode(false);
	FBranchFilter Filter;
	Filter.BoneName = TEXT("spine_01");
	Filter.BlendDepth = 3;
	Blend->Node.LayerSetup[0].BranchFilters.Add(Filter);
	Blend->Node.bMeshSpaceRotationBlend = true;
	Blend->Node.bBlendRootMotionBasedOnRootBone = true;
	Blend->NodePosX = Root->NodePosX - 330;
	Blend->NodePosY = Root->NodePosY;
	BlendCreator.Finalize();
	Added.Add(Blend);
	const UEdGraphSchema* Schema = Graph->GetSchema();
	auto Link = [Schema](UEdGraphPin* A, UEdGraphPin* B) { return A && B && Schema->TryCreateConnection(A, B); };
	Result->BreakAllPinLinks();
	const bool bConnected = Link(Original, Save->FindPin(TEXT("Pose")))
		&& Link(Base->FindPin(TEXT("Pose")), Blend->FindPin(TEXT("BasePose")))
		&& Link(Upper->FindPin(TEXT("Pose")), Slot->FindPin(TEXT("Source")))
		&& Link(Slot->FindPin(TEXT("Pose")), Blend->FindPin(TEXT("BlendPoses_0")))
		&& Link(Blend->FindPin(TEXT("Pose")), Result);
	if (bConnected)
	{
		Blueprint->TargetSkeleton->Modify();
		Blueprint->TargetSkeleton->RegisterSlotNode(TEXT("MagnesisUpperBody"));
		Blueprint->TargetSkeleton->SetSlotGroupName(TEXT("MagnesisUpperBody"), TEXT("Magnesis"));
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FCompilerResultsLog Results;
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::None, &Results);
		if (Results.NumErrors == 0 && Blueprint->Status != BS_Error)
		{
			Blueprint->MarkPackageDirty();
			Blueprint->TargetSkeleton->MarkPackageDirty();
			return true;
		}
	}
	// Never leave a disconnected output behind if the graph layout differs.
	for (UEdGraphNode* Node : Added) FBlueprintEditorUtils::RemoveNode(Blueprint, Node, true);
	Result->BreakAllPinLinks();
	Original->MakeLinkTo(Result);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	UE_LOG(LogTemp, Error, TEXT("Magnesis layer installation failed; original output restored."));
#endif
	return false;
}

bool UKzSoldierAnimationAssetUtility::InstallScanAndBombMontages()
{
#if WITH_EDITOR
	// The already installed masked slot leaves pelvis/legs on locomotion and
	// shares one montage group with Magnesis, so ability poses cannot stack.
	if (!InstallMagnesisUpperBodyLayer()) return false;
	auto Build = [](const TCHAR* Folder, const TCHAR* Name, const TArray<FString>& ClipNames,
		const TArray<FName>& Sections) -> bool
	{
		TArray<UAnimSequence*> Clips;
		for (const FString& ClipName : ClipNames)
		{
			UAnimSequence* Clip = LoadObject<UAnimSequence>(nullptr, *(FString(Folder) / ClipName));
			if (!Clip || (Clips.Num() && Clip->GetSkeleton() != Clips[0]->GetSkeleton())) return false;
			Clips.Add(Clip);
		}
		const FString Path = FString(Folder) / Name;
		UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, *Path, nullptr, LOAD_NoWarn);
		if (!Montage)
		{
			Montage = NewObject<UAnimMontage>(CreatePackage(*Path), FName(Name), RF_Public | RF_Standalone);
			FAssetRegistryModule::AssetCreated(Montage);
		}
		Montage->Modify();
		Montage->SetSkeleton(Clips[0]->GetSkeleton());
		Montage->SlotAnimTracks.Reset();
		FSlotAnimationTrack& Slot = Montage->SlotAnimTracks.AddDefaulted_GetRef();
		Slot.SlotName = TEXT("MagnesisUpperBody");
		TArray<float> Starts;
		float Position = 0.0f;
		for (UAnimSequence* Clip : Clips)
		{
			Starts.Add(Position);
			FAnimSegment Segment;
			Segment.SetAnimReference(Clip);
			Segment.StartPos = Position;
			Segment.AnimStartTime = 0.0f;
			Segment.AnimEndTime = Clip->GetPlayLength();
			Segment.AnimPlayRate = 1.0f;
			Segment.LoopingCount = 1;
			Slot.AnimTrack.AnimSegments.Add(Segment);
			Position += Clip->GetPlayLength();
		}
		Montage->SetCompositeLength(Position);
		Montage->CompositeSections.Reset();
		for (int32 Index = 0; Index < Sections.Num(); ++Index)
		{
			if (Montage->AddAnimCompositeSection(Sections[Index], Starts[Index]) == INDEX_NONE) return false;
		}
		Montage->CompositeSections[0].NextSectionName = Sections[1];
		Montage->CompositeSections[1].NextSectionName = Sections[1];
		Montage->CompositeSections[2].NextSectionName = NAME_None;
		Montage->BlendIn.SetBlendTime(0.12f);
		Montage->BlendOut.SetBlendTime(0.15f);
		Montage->PostEditChange();
		Montage->MarkPackageDirty();
		return true;
	};
	return Build(TEXT("/Game/Resources/Soldier/Anims/Abilities/Scan"), TEXT("AM_Soldier_ScanSensor"),
		{TEXT("AS_Soldier_ScanSensor_Enter"), TEXT("AS_Soldier_ScanSensor_Hold"), TEXT("AS_Soldier_ScanSensor_Exit")},
		{TEXT("Enter"), TEXT("Hold"), TEXT("Exit")})
		&& Build(TEXT("/Game/Resources/Soldier/Anims/Abilities/Bomb/Overhead"), TEXT("AM_Soldier_Bomb_Overhead"),
			{TEXT("AS_Soldier_Bomb_Overhead_Lift"), TEXT("AS_Soldier_Bomb_Overhead_Hold"), TEXT("AS_Soldier_Bomb_Overhead_Throw")},
			{TEXT("Lift"), TEXT("Hold"), TEXT("Throw")});
#else
	return false;
#endif
}
