#include "UI/KzHudWidget.h"

#include "Components/Image.h"
#include "Materials/MaterialInterface.h"

void UKzHudWidget::SetCurrentAbility(const EAbilityType AbilityType)
{
	if (!CurrentAbilityIcon)
	{
		return;
	}

	const TCHAR* MaterialPath = nullptr;

	switch (AbilityType)
	{
	case EAbilityType::Magnesis:
		MaterialPath = TEXT("/Game/Resources/UI/Common/MI_AbilityIcon_Magnesis.MI_AbilityIcon_Magnesis");
		break;
	case EAbilityType::Cryonis:
		MaterialPath = TEXT("/Game/Resources/UI/Common/MI_AbilityIcon_Cryonis.MI_AbilityIcon_Cryonis");
		break;
	case EAbilityType::Stasis:
		MaterialPath = TEXT("/Game/Resources/UI/Common/MI_AbilityIcon_Stasis.MI_AbilityIcon_Stasis");
		break;
	case EAbilityType::RemoteBombSphere:
		MaterialPath = TEXT("/Game/Resources/UI/Common/MI_AbilityIcon_RemoteBombSphere.MI_AbilityIcon_RemoteBombSphere");
		break;
	case EAbilityType::RemoteBombCube:
		MaterialPath = TEXT("/Game/Resources/UI/Common/MI_AbilityIcon_RemoteBombCube.MI_AbilityIcon_RemoteBombCube");
		break;
	default:
		CurrentAbilityIcon->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	UMaterialInterface* IconMaterial = LoadObject<UMaterialInterface>(nullptr, MaterialPath);

	if (!IconMaterial)
	{
		CurrentAbilityIcon->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	CurrentAbilityIcon->SetBrushFromMaterial(IconMaterial);
	CurrentAbilityIcon->SetVisibility(ESlateVisibility::Visible);
}
