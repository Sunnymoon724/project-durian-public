#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/KzBaseWidget.h"
#include "KzHudWidget.generated.h"

UCLASS(Blueprintable)
class DURIAN_API UKzHudWidget : public UKzBaseWidget
{
	GENERATED_BODY()

public:
	static TSubclassOf<UKzBaseWidget> GetWidgetClass();

protected:
	virtual void NativeConstruct() override;
};
