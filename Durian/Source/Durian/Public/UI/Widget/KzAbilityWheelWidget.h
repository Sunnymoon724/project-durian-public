#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/KzBaseWidget.h"
#include "KzAbilityWheelWidget.generated.h"

/**
 * Base widget for the ability selection wheel's state and display lifetime.
 * Slot layout and visual effects are handled by WBP_AbilityWheel.
 */
UCLASS(Blueprintable)
class DURIAN_API UKzAbilityWheelWidget : public UKzBaseWidget
{
	GENERATED_BODY()

public:
	static TSubclassOf<UKzBaseWidget> GetWidgetClass();
	class UKzAbilityWheelViewModel* GetViewModel() const;

protected:
	virtual void NativeConstruct() override;
};
