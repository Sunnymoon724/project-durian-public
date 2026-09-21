#pragma once

#include "CoreMinimal.h"
#include "Enums/PlayerEnums.h"
#include "UI/KzBaseWidget.h"
#include "Components/Image.h"
#include "KzAbilityWheelWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityWheelAbilityChanged, EAbilityType, AbilityType);

/**
 * Base widget for the ability selection wheel's state and display lifetime.
 * Slot layout and visual effects are handled by WBP_AbilityWheel.
 */
UCLASS(Blueprintable)
class DURIAN_API UKzAbilityWheelWidget : public UKzBaseWidget
{
	GENERATED_BODY()

public:
	UKzAbilityWheelWidget();

	UFUNCTION(BlueprintCallable, Category = "Ability Wheel")
	void OpenWheel();

	UFUNCTION(BlueprintCallable, Category = "Ability Wheel")
	void CloseWheel();

	UFUNCTION(BlueprintCallable, Category = "Ability Wheel")
	void SetHighlightedSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Ability Wheel")
	void SetHighlightedAbility(EAbilityType AbilityType);

	UFUNCTION(BlueprintCallable, Category = "Ability Wheel")
	void SetSelectionDirection(FVector2D Direction);

	UFUNCTION(BlueprintCallable, Category = "Ability Wheel")
	void AddSelectionDirection(FVector2D DirectionDelta);

	UFUNCTION(BlueprintCallable, Category = "Ability Wheel")
	void ConfirmSelection();

	UFUNCTION(BlueprintPure, Category = "Ability Wheel")
	int32 GetHighlightedSlot() const { return HighlightedSlot; }

	UFUNCTION(BlueprintPure, Category = "Ability Wheel")
	EAbilityType GetHighlightedAbility() const;

	UFUNCTION(BlueprintPure, Category = "Ability Wheel")
	TArray<EAbilityType> GetAbilitySlots() const { return AbilitySlotArray; }

	UFUNCTION(BlueprintPure, Category = "Ability Wheel")
	bool IsWheelOpen() const { return bWheelOpen; }

	UPROPERTY(BlueprintAssignable, Category = "Ability Wheel")
	FOnAbilityWheelAbilityChanged OnHighlightedAbilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Ability Wheel")
	FOnAbilityWheelAbilityChanged OnAbilityConfirmed;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Wheel")
	TArray<EAbilityType> AbilitySlotArray;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Wheel")
	int32 HighlightedSlot = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ability Wheel")
	bool bWheelOpen = false;

	FVector2D AccumulatedSelectionDirection = FVector2D::ZeroVector;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SelectOverlay;

	virtual void NativeConstruct() override;

private:
	void BroadcastHighlightedAbility() const;
	void UpdateSelectOverlay() const;
};
