#include "Abilities/AbilityReactionComponent.h"

#include "Abilities/AbilityModeSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UAbilityReactionComponent::UAbilityReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	TopScanOverlayMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Resources/VFX/Magnet/Materials/M_MagnetTargetTopScanOverlay.M_MagnetTargetTopScanOverlay")));
}

void UAbilityReactionComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->RegisterAbilityModeListener(this);
	}
}

void UAbilityReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		GameInstance->GetSubsystem<UAbilityModeSubsystem>()->UnregisterAbilityModeListener(this);
	}
	SetMagnetStencilEnabled(false);
	for (UStaticMeshComponent* Overlay : TopScanOverlays) if (IsValid(Overlay)) Overlay->DestroyComponent();
	TopScanOverlays.Empty();
	TopScanMaterials.Empty();
	Super::EndPlay(EndPlayReason);
}

TArray<EAbilityMode> UAbilityReactionComponent::GetSupportedAbilityModes_Implementation() const
{
	switch (ReactionType)
	{
	case EAbilityReactionType::Normal: return { EAbilityMode::Magnet, EAbilityMode::Ice, EAbilityMode::TimeLock, EAbilityMode::Bomb };
	case EAbilityReactionType::MagnetTarget: return { EAbilityMode::Magnet };
	case EAbilityReactionType::TimeLockTarget: return { EAbilityMode::TimeLock };
	case EAbilityReactionType::Water: return { EAbilityMode::Ice };
	default: return {};
	}
}

void UAbilityReactionComponent::OnAbilityModeChanged_Implementation(const EAbilityMode Mode, const bool bEnabled)
{
	if (ReactionType == EAbilityReactionType::Normal)
	{
		SetTopScanEnabled(bEnabled, Mode);
	}
	else if (ReactionType == EAbilityReactionType::MagnetTarget && Mode == EAbilityMode::Magnet)
	{
		SetMagnetStencilEnabled(bEnabled);
		SetTopScanEnabled(bEnabled, Mode);
		if (bEnabled)
		{
			// Targets keep the same local top-face projection as normal blocks,
			// but use a much brighter warm color so they read as interactable.
			for (UMaterialInstanceDynamic* Material : TopScanMaterials)
			{
				if (Material) Material->SetVectorParameterValue(TEXT("ScanColor"), FLinearColor(4.0f, 0.18f, 0.01f));
			}
		}
	}
}

void UAbilityReactionComponent::SetMagnetStencilEnabled(const bool bEnabled) const
{
	TArray<UPrimitiveComponent*> Components;
	GetOwner()->GetComponents(Components);
	for (UPrimitiveComponent* Component : Components)
	{
		if (Component && !TopScanOverlays.Contains(Cast<UStaticMeshComponent>(Component)))
		{
			Component->SetRenderCustomDepth(bEnabled);
			Component->SetCustomDepthStencilValue(bEnabled ? 1 : 0);
		}
	}
}

void UAbilityReactionComponent::SetTopScanEnabled(const bool bEnabled, const EAbilityMode Mode)
{
	if (bEnabled) CreateTopScanOverlays();
	const FLinearColor Color = GetModeColor(Mode);
	for (int32 Index = 0; Index < TopScanOverlays.Num(); ++Index)
	{
		if (UStaticMeshComponent* Overlay = TopScanOverlays[Index])
		{
			// Existing overlays survive ability toggles, so refresh their separation
			// as well as newly created overlays. Scaling does not separate a flat
			// plane, so use a small local-Z offset as the actual depth separation.
			Overlay->SetRelativeScale3D(FVector(1.005f));
			Overlay->SetRelativeLocation(FVector(0.0f, 0.0f, 0.5f));
			Overlay->SetHiddenInGame(!bEnabled, true);
			Overlay->SetVisibility(bEnabled, true);
		}
		if (TopScanMaterials.IsValidIndex(Index) && TopScanMaterials[Index]) TopScanMaterials[Index]->SetVectorParameterValue(TEXT("ScanColor"), Color);
	}
}

void UAbilityReactionComponent::CreateTopScanOverlays()
{
	if (TopScanOverlays.Num() || !GetOwner()) return;
	UMaterialInterface* Material = TopScanOverlayMaterial.LoadSynchronous();
	if (!Material) return;
	TArray<UStaticMeshComponent*> Sources; GetOwner()->GetComponents(Sources);
	for (UStaticMeshComponent* Source : Sources)
	{
		if (!Source || !Source->GetStaticMesh()) continue;
		UStaticMeshComponent* Overlay = NewObject<UStaticMeshComponent>(GetOwner(), NAME_None, RF_Transient);
		GetOwner()->AddInstanceComponent(Overlay); Overlay->SetStaticMesh(Source->GetStaticMesh()); Overlay->SetupAttachment(Source);
		// This is a separate translucent shell. Keep it far enough from the source
		// mesh to avoid depth-buffer Z-fighting while the camera moves.
		Overlay->SetRelativeScale3D(FVector(1.005f)); Overlay->SetRelativeLocation(FVector(0.0f, 0.0f, 0.5f)); Overlay->SetCollisionEnabled(ECollisionEnabled::NoCollision); Overlay->SetCastShadow(false); Overlay->SetRenderCustomDepth(false);
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
		for (int32 Slot = 0; Slot < Source->GetNumMaterials(); ++Slot) Overlay->SetMaterial(Slot, DynamicMaterial);
		Overlay->SetHiddenInGame(true, true); Overlay->RegisterComponent();
		TopScanOverlays.Add(Overlay); TopScanMaterials.Add(DynamicMaterial);
	}
}

FLinearColor UAbilityReactionComponent::GetModeColor(const EAbilityMode Mode) const
{
	switch (Mode)
	{
	case EAbilityMode::Ice: return FLinearColor(0.08f, 0.75f, 1.0f);
	case EAbilityMode::TimeLock: return FLinearColor(1.0f, 0.72f, 0.05f);
	case EAbilityMode::Bomb: return FLinearColor(1.0f, 0.25f, 0.05f);
	case EAbilityMode::Magnet:
	default: return FLinearColor(1.0f, 0.035f, 0.045f);
	}
}
