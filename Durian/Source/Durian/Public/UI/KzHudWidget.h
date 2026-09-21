#pragma once

#include "CoreMinimal.h"
#include "Enums/PlayerEnums.h"
#include "UI/KzBaseWidget.h"
#include "KzHudWidget.generated.h"

class UImage;

UCLASS(Blueprintable)
class DURIAN_API UKzHudWidget : public UKzBaseWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetCurrentAbility(EAbilityType AbilityType);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CurrentAbilityIcon;
};
