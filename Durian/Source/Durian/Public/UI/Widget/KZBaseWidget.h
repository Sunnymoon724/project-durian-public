#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enums/UIEnums.h"
#include "KzBaseWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class DURIAN_API UKzBaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	EUIPriorityType GetPriorityType() const { return PriorityType; }

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess=true))
	EUIPriorityType PriorityType = EUIPriorityType::Middle;
};
