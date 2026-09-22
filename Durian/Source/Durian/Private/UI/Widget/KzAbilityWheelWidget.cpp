#include "UI/KzAbilityWheelWidget.h"

#include "Components/Image.h"
#include "Engine/Engine.h"

UKzAbilityWheelWidget::UKzAbilityWheelWidget()
{
	AbilitySlotArray =
	{
		EAbilityType::Magnesis,
		EAbilityType::Cryonis,
		EAbilityType::Stasis,
		EAbilityType::RemoteBombSphere,
		EAbilityType::RemoteBombCube
	};

	HighlightedSlot = 0;
}

void UKzAbilityWheelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bWheelOpen = false;

	SetVisibility(ESlateVisibility::Collapsed);
}

void UKzAbilityWheelWidget::OpenWheel()
{
	if (AbilitySlotArray.IsEmpty())
	{
		return;
	}

	bWheelOpen = true;
	AccumulatedSelectionDirection = FVector2D::ZeroVector;

	SetVisibility(ESlateVisibility::Visible);

	UpdateSelectOverlay();
	BroadcastHighlightedAbility();
}

void UKzAbilityWheelWidget::CloseWheel()
{
	bWheelOpen = false;
	AccumulatedSelectionDirection = FVector2D::ZeroVector;

	UpdateSelectOverlay();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UKzAbilityWheelWidget::SetHighlightedSlot(const int32 SlotIndex)
{
	if (AbilitySlotArray.IsEmpty())
	{
		return;
	}

	const int32 NewSlot = FMath::Clamp(SlotIndex, 0, AbilitySlotArray.Num() - 1);

	if (HighlightedSlot == NewSlot)
	{
		return;
	}

	HighlightedSlot = NewSlot;

	UpdateSelectOverlay();
	BroadcastHighlightedAbility();
}

void UKzAbilityWheelWidget::SetHighlightedAbility(const EAbilityType AbilityType)
{
	const int32 SlotIndex = AbilitySlotArray.IndexOfByKey(AbilityType);

	if (SlotIndex != INDEX_NONE)
	{
		SetHighlightedSlot(SlotIndex);
	}
}

void UKzAbilityWheelWidget::SetSelectionDirection(const FVector2D Direction)
{
	if (AbilitySlotArray.IsEmpty() || Direction.IsNearlyZero())
	{
		return;
	}

	// Use the top of the screen as slot 0 and place the remaining slots clockwise.
	const FVector2D CorrectedDirection = FVector2D(Direction.X,-Direction.Y);
	const float AngleFromTop = FMath::Fmod(FMath::Atan2(CorrectedDirection.X, CorrectedDirection.Y) + 2.0f * PI, 2.0f * PI);
	const float SlotAngle = 2.0f * PI / static_cast<float>(AbilitySlotArray.Num());
	const int32 SlotIndex = FMath::RoundToInt(AngleFromTop / SlotAngle) % AbilitySlotArray.Num();

	SetHighlightedSlot(SlotIndex);
}

void UKzAbilityWheelWidget::AddSelectionDirection(const FVector2D DirectionDelta)
{
	if (!bWheelOpen || DirectionDelta.IsNearlyZero())
	{
		return;
	}

	AccumulatedSelectionDirection += DirectionDelta;

	if (!AccumulatedSelectionDirection.IsNearlyZero())
	{
		SetSelectionDirection(AccumulatedSelectionDirection);
	}
}

void UKzAbilityWheelWidget::ConfirmSelection()
{
	if (!bWheelOpen || AbilitySlotArray.IsEmpty())
	{
		return;
	}

	OnAbilityConfirmed.Broadcast(GetHighlightedAbility());

	CloseWheel();
}

EAbilityType UKzAbilityWheelWidget::GetHighlightedAbility() const
{
	return AbilitySlotArray.IsValidIndex(HighlightedSlot) ? AbilitySlotArray[HighlightedSlot] : EAbilityType::None;
}

void UKzAbilityWheelWidget::BroadcastHighlightedAbility() const
{
	OnHighlightedAbilityChanged.Broadcast(GetHighlightedAbility());
}

void UKzAbilityWheelWidget::UpdateSelectOverlay() const
{
	if (!SelectOverlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ability Wheel overlay update skipped: SelectOverlay is not bound."));

		return;
	}

	SelectOverlay->SetVisibility(bWheelOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (!bWheelOpen)
	{
		SelectOverlay->SetRenderTransformAngle(0.0f);
		SelectOverlay->SetRenderTranslation(FVector2D::ZeroVector);

		return;
	}

	const float RotationStep = 360.0f / static_cast<float>(AbilitySlotArray.Num());
	const float TargetAngle = RotationStep * static_cast<float>(HighlightedSlot);

	SelectOverlay->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	SelectOverlay->SetRenderTransformAngle(TargetAngle);
	SelectOverlay->SetRenderTranslation(FVector2D::ZeroVector);
}
