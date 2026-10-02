#include "Abilities/Components/AbilityReactionComponent.h"

#include "Abilities/Core/AbilityModeSubsystem.h"
#include "Abilities/Core/AbilityModeVisualProfile.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
	EAbilityReactionType GetTargetReactionType(const EAbilityVisualMode Mode)
	{
		switch (Mode)
		{
		case EAbilityVisualMode::Magnesis: return EAbilityReactionType::MagnesisTarget;
		case EAbilityVisualMode::Cryonis: return EAbilityReactionType::CryonicTarget;
		case EAbilityVisualMode::Stasis: return EAbilityReactionType::StasisTarget;
		default: return EAbilityReactionType::Normal;
		}
	}

}

UAbilityReactionComponent::UAbilityReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	TargetSurfaceHighlightMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Resources/VFX/AbilityMode/Materials/M_AbilityModeTargetSurface.M_AbilityModeTargetSurface")));
}

void UAbilityReactionComponent::SetMagnesisHeld(const bool bIsHeld)
{
	if (bIsMagnesisHeld == bIsHeld)
	{
		return;
	}

	bIsMagnesisHeld = bIsHeld;
	OnMagnesisHeldStateChanged.Broadcast(bIsMagnesisHeld);
}

void UAbilityReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;

	if (GameInstance)
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->RegisterAbilityModeListener(this);
	}
}

void UAbilityReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;

	if (GameInstance)
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->UnregisterAbilityModeListener(this);
	}

	SetTargetStencilEnabled(false);

	for (UStaticMeshComponent* Overlay : TargetSurfaceHighlightOverlays)
	{
		if (IsValid(Overlay))
		{
			Overlay->DestroyComponent();
		}
	}

	TargetSurfaceHighlightOverlays.Empty();
	TargetSurfaceHighlightMaterials.Empty();

	Super::EndPlay(EndPlayReason);
}

TArray<EAbilityVisualMode> UAbilityReactionComponent::GetSupportedAbilityModes_Implementation() const
{
	return { EAbilityVisualMode::Magnesis, EAbilityVisualMode::Cryonis, EAbilityVisualMode::Stasis };
}

void UAbilityReactionComponent::OnAbilityModeChanged_Implementation(const EAbilityVisualMode Mode, const bool bEnabled)
{
	ActiveMode = bEnabled ? Mode : EAbilityVisualMode::None;

	if (!bEnabled)
	{
		bAimedTarget = false;
	}

	const bool bIsMatchingTarget = ReactionType == GetTargetReactionType(Mode);

	if (bIsMatchingTarget)
	{
		SetTargetStencilEnabled(bEnabled && bAimedTarget);
		SetTargetSurfaceHighlightEnabled(bEnabled, Mode);
	}
	else
	{
		SetTargetStencilEnabled(false);
		SetTargetSurfaceHighlightEnabled(false, Mode);
	}
}

void UAbilityReactionComponent::SetAimedTarget(const bool bAimed)
{
	bAimedTarget = bAimed;

	const bool bIsMatchingTarget = ReactionType == GetTargetReactionType(ActiveMode);

	if (ActiveMode != EAbilityVisualMode::None && bIsMatchingTarget)
	{
		SetTargetStencilEnabled(bAimedTarget);
	
		if (bAimedTarget && GetOwner())
		{
			TArray<UPrimitiveComponent*> ComponentArray;

			GetOwner()->GetComponents(ComponentArray);

			for (UPrimitiveComponent* Component : ComponentArray)
			{
				if (Component)
				{
					Component->SetCustomDepthStencilValue(2);
				}
			}
		}

		SetTargetSurfaceHighlightEnabled(true, ActiveMode);
	}
}

void UAbilityReactionComponent::SetTargetStencilEnabled(const bool bEnabled) const
{
	TArray<UPrimitiveComponent*> ComponentArray;

	GetOwner()->GetComponents(ComponentArray);

	for (UPrimitiveComponent* Component : ComponentArray)
	{
		if (Component)
		{
			Component->SetRenderCustomDepth(false);
			Component->SetCustomDepthStencilValue(0);
		}
	}
}

void UAbilityReactionComponent::SetTargetSurfaceHighlightEnabled(const bool bEnabled, const EAbilityVisualMode Mode)
{
	if (bEnabled)
	{
		CreateTargetSurfaceHighlightOverlays();
	}

	const FAbilityModeVisualProfile& VisualProfile = FAbilityModeVisualProfiles::Get(Mode);
	const FAbilityModeVisualCommonProfile& CommonProfile = FAbilityModeVisualProfiles::GetCommon();

	for (int32 Index = 0; Index < TargetSurfaceHighlightOverlays.Num(); ++Index)
	{
		if (UStaticMeshComponent* Overlay = TargetSurfaceHighlightOverlays[Index])
		{
			Overlay->SetHiddenInGame(!bEnabled, true);
			Overlay->SetVisibility(bEnabled, true);
		}

		if (TargetSurfaceHighlightMaterials.IsValidIndex(Index) && TargetSurfaceHighlightMaterials[Index])
		{
			TargetSurfaceHighlightMaterials[Index]->SetVectorParameterValue(TEXT("TargetEdgeColor"), bAimedTarget ? VisualProfile.TargetEdgeColor : VisualProfile.CandidateGlowColor);
			TargetSurfaceHighlightMaterials[Index]->SetVectorParameterValue(TEXT("TargetFillColor"), bAimedTarget ? VisualProfile.TargetScanColor : VisualProfile.CandidateGradeColor);
			TargetSurfaceHighlightMaterials[Index]->SetScalarParameterValue(TEXT("TargetEdgeOpacity"), CommonProfile.TargetEdgeOpacity);
		}
	}
}

void UAbilityReactionComponent::CreateTargetSurfaceHighlightOverlays()
{
	if (TargetSurfaceHighlightOverlays.Num() || !GetOwner())
	{
		return;
	}

	UMaterialInterface* Material = TargetSurfaceHighlightMaterial.LoadSynchronous();
	const FAbilityModeVisualCommonProfile& CommonProfile = FAbilityModeVisualProfiles::GetCommon();

	if (!Material)
	{
		return;
	}

	TArray<UStaticMeshComponent*> SourceArray;

	GetOwner()->GetComponents(SourceArray);

	for (UStaticMeshComponent* Source : SourceArray)
	{
		if (!Source || !Source->GetStaticMesh() || TargetSurfaceHighlightOverlays.Contains(Source))
		{
			continue;
		}

		UStaticMeshComponent* Overlay = NewObject<UStaticMeshComponent>(GetOwner(), NAME_None, RF_Transient);

		GetOwner()->AddInstanceComponent(Overlay);

		Overlay->SetStaticMesh(Source->GetStaticMesh());
		Overlay->SetupAttachment(Source);
		Overlay->SetRelativeScale3D(FVector(CommonProfile.ScanTargetTopOverlayScale));
		Overlay->SetRelativeLocation(FVector(0.0f, 0.0f, CommonProfile.ScanTargetTopOverlayZOffset));
		Overlay->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Overlay->SetCastShadow(false);
		Overlay->SetRenderCustomDepth(false);

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);

		for (int32 Slot = 0; Slot < Source->GetNumMaterials(); ++Slot)
		{
			Overlay->SetMaterial(Slot, DynamicMaterial);
		}

		Overlay->SetHiddenInGame(true, true);
		Overlay->RegisterComponent();

		TargetSurfaceHighlightOverlays.Add(Overlay);
		TargetSurfaceHighlightMaterials.Add(DynamicMaterial);
	}
}
