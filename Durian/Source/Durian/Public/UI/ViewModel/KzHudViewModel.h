#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/KzViewModelBase.h"
#include "Enums/PlayerEnums.h"
#include "Styling/SlateBrush.h"
#include "KzHudViewModel.generated.h"

class AKzPlayerCharacter;
class APawn;

UCLASS(BlueprintType)
class DURIAN_API UKzHudViewModel : public UKzViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override;

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Ability", meta=(AllowPrivateAccess=true))
	FSlateBrush CurrentAbilityBrush;

	TWeakObjectPtr<AKzPlayerCharacter> PlayerCharacter;
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	void RefreshCurrentAbilityBrush();
	void HandleAbilityChanged(EAbilityType AbilityType);
};
