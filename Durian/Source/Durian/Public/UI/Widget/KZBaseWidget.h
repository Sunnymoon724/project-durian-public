#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enums/UIEnums.h"
#include "UI/KzViewModelBase.h"
#include "KzBaseWidget.generated.h"

USTRUCT(BlueprintType)
struct FViewModelBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|MVVM")
	FName ContextName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|MVVM")
	TSubclassOf<UKzViewModelBase> ViewModelClass;
};

UCLASS(Abstract, Blueprintable)
class DURIAN_API UKzBaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	const TArray<FViewModelBinding>& GetViewModelBindingArray() const { return ViewModelBindingArray; }
	EUIPriorityType GetPriorityType() const { return PriorityType; }

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI|MVVM", meta=(AllowPrivateAccess=true))
	TArray<FViewModelBinding> ViewModelBindingArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess=true))
	EUIPriorityType PriorityType = EUIPriorityType::Middle;
};
