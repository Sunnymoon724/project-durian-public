#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/KzViewModelBase.h"
#include "Enums/PlayerEnums.h"
#include "Components/SlateWrapperTypes.h"
#include "KzAbilityWheelViewModel.generated.h"

class AKzPlayerCharacter;
class APawn;

UCLASS(BlueprintType)
class DURIAN_API UKzAbilityWheelViewModel : public UKzViewModelBase
{
	GENERATED_BODY()

public:
	virtual void Initialize() override;

	UFUNCTION(BlueprintCallable)
	void OpenWheel();

	UFUNCTION(BlueprintCallable)
	void CloseWheel();

	UFUNCTION(BlueprintCallable)
	void ConfirmSelection();

	UFUNCTION(BlueprintCallable)
	void SetSelectedAbility(EAbilityType AbilityType);

	UFUNCTION(BlueprintCallable)
	void AddSelectionDirection(FVector2D DirectionDelta);

	UFUNCTION(BlueprintPure, FieldNotify)
	bool GetIsOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintPure, FieldNotify)
	ESlateVisibility GetWheelVisibility() const { return WheelVisibility; }

	UFUNCTION(BlueprintPure, FieldNotify)
	EAbilityType GetSelectedAbility() const { return SelectedAbility; }

	UFUNCTION(BlueprintPure, FieldNotify)
	float GetSelectionAngle() const { return SelectionAngle; }

private:
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Ability Wheel", meta=(AllowPrivateAccess=true))
	bool bIsOpen = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Ability Wheel", meta=(AllowPrivateAccess=true))
	ESlateVisibility WheelVisibility = ESlateVisibility::Collapsed;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Ability Wheel", meta=(AllowPrivateAccess=true))
	EAbilityType SelectedAbility = EAbilityType::None;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category="Ability Wheel", meta=(AllowPrivateAccess=true))
	float SelectionAngle = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category="Ability Wheel")
	TArray<EAbilityType> AbilitySlotArray;

	TWeakObjectPtr<AKzPlayerCharacter> PlayerCharacter;
	FVector2D AccumulatedSelectionDirection = FVector2D::ZeroVector;
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	void UpdateSelectionAngle();
};
