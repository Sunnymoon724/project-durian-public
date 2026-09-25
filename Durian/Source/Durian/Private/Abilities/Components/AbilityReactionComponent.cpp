#include "Abilities/Components/AbilityReactionComponent.h"

#include "Abilities/Core/AbilityModeSubsystem.h"
#include "Abilities/Core/AbilityModeVisualProfile.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UAbilityReactionComponent::UAbilityReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	TopScanOverlayMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Resources/VFX/AbilityMode/Materials/M_AbilityModeTopScanOverlay.M_AbilityModeTopScanOverlay")));
	TargetSurfaceHighlightMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Resources/VFX/AbilityMode/Materials/M_AbilityModeTargetSurface.M_AbilityModeTargetSurface")));
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

	for (UStaticMeshComponent* Overlay : TopScanOverlays)
	{
		if (IsValid(Overlay))
		{
			Overlay->DestroyComponent();
		}
	}

	for (UStaticMeshComponent* Overlay : TargetSurfaceHighlightOverlays)
	{
		if (IsValid(Overlay))
		{
			Overlay->DestroyComponent();
		}
	}

	TopScanOverlays.Empty();
	TopScanMaterials.Empty();

	TargetSurfaceHighlightOverlays.Empty();
	TargetSurfaceHighlightMaterials.Empty();

	Super::EndPlay(EndPlayReason);
}

TArray<EAbilityVisualMode> UAbilityReactionComponent::GetSupportedAbilityModes_Implementation() const
{
	switch (ReactionType)
	{
		case EAbilityReactionType::Normal:
			return { EAbilityVisualMode::Magnesis, EAbilityVisualMode::Cryonis, EAbilityVisualMode::Stasis };
		case EAbilityReactionType::MagnesisTarget:
		case EAbilityReactionType::StasisTarget:
		case EAbilityReactionType::CryonicTarget:
			return { EAbilityVisualMode::Magnesis, EAbilityVisualMode::Cryonis, EAbilityVisualMode::Stasis };
		default:
			UE_LOG(LogTemp, Log, TEXT("Not supported in %s."), *UEnum::GetValueAsString(ReactionType));
			return {};
	}
}

void UAbilityReactionComponent::OnAbilityModeChanged_Implementation(const EAbilityVisualMode Mode, const bool bEnabled)
{
	ActiveMode = bEnabled ? Mode : EAbilityVisualMode::None;

	if (!bEnabled)
	{
		bAimedTarget = false;
	}

	const bool bIsMatchingTarget = (ReactionType == EAbilityReactionType::MagnesisTarget && Mode == EAbilityVisualMode::Magnesis) || (ReactionType == EAbilityReactionType::StasisTarget && Mode == EAbilityVisualMode::Stasis) || (ReactionType == EAbilityReactionType::CryonicTarget && Mode == EAbilityVisualMode::Cryonis);

	if (bIsMatchingTarget)
	{
		SetTargetStencilEnabled(bEnabled);
		if (bEnabled && bAimedTarget)
		{
			SetAimedTarget(true);
		}
		SetTargetSurfaceHighlightEnabled(bEnabled, Mode);
	}
	else
	{
		SetTargetStencilEnabled(false);
		SetTargetSurfaceHighlightEnabled(false, Mode);
		SetTopScanEnabled(bEnabled, Mode);
	}
}

void UAbilityReactionComponent::SetAimedTarget(const bool bAimed)
{
	bAimedTarget = bAimed;

	const bool bIsMatchingTarget =(ReactionType == EAbilityReactionType::MagnesisTarget && ActiveMode == EAbilityVisualMode::Magnesis) || (ReactionType == EAbilityReactionType::StasisTarget && ActiveMode == EAbilityVisualMode::Stasis) || (ReactionType == EAbilityReactionType::CryonicTarget && ActiveMode == EAbilityVisualMode::Cryonis);

	if (ActiveMode != EAbilityVisualMode::None && bIsMatchingTarget)
	{
		SetTargetStencilEnabled(true);
	
		if (bAimedTarget && GetOwner())
		{
			TArray<UPrimitiveComponent*> ComponentArray;

			GetOwner()->GetComponents(ComponentArray);

			for (UPrimitiveComponent* Component : ComponentArray)
			{
				if (Component && !Component->ComponentHasTag(TEXT("StasisFeedback")) && !TopScanOverlays.Contains(Cast<UStaticMeshComponent>(Component)))
				{
					Component->SetCustomDepthStencilValue(2);
				}
			}
		}

		SetTargetSurfaceHighlightEnabled(true, ActiveMode);
	}
}

bool UAbilityReactionComponent::IsTargetReaction() const
{
	return ReactionType != EAbilityReactionType::Normal;
}

void UAbilityReactionComponent::SetTargetStencilEnabled(const bool bEnabled) const
{
	TArray<UPrimitiveComponent*> ComponentArray;

	GetOwner()->GetComponents(ComponentArray);

	for (UPrimitiveComponent* Component : ComponentArray)
	{
		if (Component && !Component->ComponentHasTag(TEXT("StasisFeedback")) && !TopScanOverlays.Contains(Cast<UStaticMeshComponent>(Component)))
		{
			Component->SetRenderCustomDepth(bEnabled);
			Component->SetCustomDepthStencilValue(bEnabled ? 1 : 0);
		}
	}
}

void UAbilityReactionComponent::SetTopScanEnabled(const bool bEnabled, const EAbilityVisualMode Mode)
{
	if (bEnabled)
	{
		CreateTopScanOverlays();
	}

	const FAbilityModeVisualProfile& VisualProfile = FAbilityModeVisualProfiles::Get(Mode);
	const FLinearColor Color = VisualProfile.ScanColor;

	FVector2D ScanDirection(1.0f, 0.0f);

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;

	if (GameInstance)
	{
		if (const UAbilityModeSubsystem* AbilityModeSubsystem = GameInstance->GetSubsystem<UAbilityModeSubsystem>())
		{
			ScanDirection = AbilityModeSubsystem->GetModeScanDirection();
		}
	}

	for (int32 Index = 0; Index < TopScanOverlays.Num(); ++Index)
	{
		if (UStaticMeshComponent* Overlay = TopScanOverlays[Index])
		{
			Overlay->SetRelativeScale3D(FVector(1.005f));
			Overlay->SetRelativeLocation(FVector(0.0f, 0.0f, 0.5f));
			Overlay->SetHiddenInGame(!bEnabled, true);
			Overlay->SetVisibility(bEnabled, true);
		}

		if (TopScanMaterials.IsValidIndex(Index) && TopScanMaterials[Index])
		{
			TopScanMaterials[Index]->SetVectorParameterValue(TEXT("ScanColor"), Color);
			TopScanMaterials[Index]->SetScalarParameterValue(TEXT("ScanAngle"), FMath::Atan2(ScanDirection.Y, ScanDirection.X));
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
			TargetSurfaceHighlightMaterials[Index]->SetVectorParameterValue(TEXT("TargetEdgeColor"),bAimedTarget ? VisualProfile.TargetEdgeColor : VisualProfile.CandidateGlowColor);
			TargetSurfaceHighlightMaterials[Index]->SetScalarParameterValue(TEXT("TargetEdgeOpacity"), CommonProfile.TargetEdgeOpacity);
		}
	}
}

void UAbilityReactionComponent::CreateTopScanOverlays()
{
	if (TopScanOverlays.Num() || !GetOwner())
	{
		return;
	}

	UMaterialInterface* Material = TopScanOverlayMaterial.LoadSynchronous();

	if (!Material)
	{
		return;
	}

	TArray<UStaticMeshComponent*> SourceArray;

	GetOwner()->GetComponents(SourceArray);

	for (UStaticMeshComponent* Source : SourceArray)
	{
		if (!Source || !Source->GetStaticMesh() || Source->ComponentHasTag(TEXT("StasisFeedback")) || TopScanOverlays.Contains(Source) || TargetSurfaceHighlightOverlays.Contains(Source))
		{
			continue;
		}

		UStaticMeshComponent* Overlay = NewObject<UStaticMeshComponent>(GetOwner(), NAME_None, RF_Transient);

		GetOwner()->AddInstanceComponent(Overlay);

		Overlay->SetStaticMesh(Source->GetStaticMesh());
		Overlay->SetupAttachment(Source);
		Overlay->SetRelativeScale3D(FVector(1.005f));
		Overlay->SetRelativeLocation(FVector(0.0f, 0.0f, 0.5f));
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

		TopScanOverlays.Add(Overlay);
		TopScanMaterials.Add(DynamicMaterial);
	}
}

void UAbilityReactionComponent::CreateTargetSurfaceHighlightOverlays()
{
	if (TargetSurfaceHighlightOverlays.Num() || !GetOwner())
	{
		return;
	}

	UMaterialInterface* Material = TargetSurfaceHighlightMaterial.LoadSynchronous();

	if (!Material)
	{
		return;
	}

	TArray<UStaticMeshComponent*> SourceArray;

	GetOwner()->GetComponents(SourceArray);

	for (UStaticMeshComponent* Source : SourceArray)
	{
		if (!Source || !Source->GetStaticMesh() || Source->ComponentHasTag(TEXT("StasisFeedback")) || TopScanOverlays.Contains(Source) || TargetSurfaceHighlightOverlays.Contains(Source))
		{
			continue;
		}

		UStaticMeshComponent* Overlay = NewObject<UStaticMeshComponent>(GetOwner(), NAME_None, RF_Transient);

		GetOwner()->AddInstanceComponent(Overlay);

		Overlay->SetStaticMesh(Source->GetStaticMesh());
		Overlay->SetupAttachment(Source);
		Overlay->SetRelativeScale3D(FVector(1.012f));
		Overlay->SetRelativeLocation(FVector(0.0f, 0.0f, 0.01f));
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
