#include "UI/ViewModel/KzAbilityWheelViewModel.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/PlayerController.h"

void UKzAbilityWheelViewModel::Initialize()
{
	Super::Initialize();

	AbilitySlotArray = { EAbilityType::Magnesis, EAbilityType::Cryonis, EAbilityType::Stasis, EAbilityType::RemoteBombSphere, EAbilityType::RemoteBombCube };

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;

	if (PlayerController)
	{
		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &UKzAbilityWheelViewModel::HandlePossessedPawnChanged);
		HandlePossessedPawnChanged(nullptr, PlayerController->GetPawn());
	}
}

void UKzAbilityWheelViewModel::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	PlayerCharacter = Cast<AKzPlayerCharacter>(NewPawn);

	if (PlayerCharacter.IsValid())
	{
		SetSelectedAbility(PlayerCharacter->GetCurrentAbilityType());
	}
}

void UKzAbilityWheelViewModel::OpenWheel()
{
	if (!PlayerCharacter.IsValid())
	{
		return;
	}

	SetSelectedAbility(PlayerCharacter->GetCurrentAbilityType());

	UE_MVVM_SET_PROPERTY_VALUE(bIsOpen, true);
	UE_MVVM_SET_PROPERTY_VALUE(WheelVisibility, ESlateVisibility::Visible);
}

void UKzAbilityWheelViewModel::CloseWheel()
{
	UE_MVVM_SET_PROPERTY_VALUE(bIsOpen, false);
	UE_MVVM_SET_PROPERTY_VALUE(WheelVisibility, ESlateVisibility::Collapsed);

	AccumulatedSelectionDirection = FVector2D::ZeroVector;
}

void UKzAbilityWheelViewModel::ConfirmSelection()
{
	if (PlayerCharacter.IsValid() && SelectedAbility != EAbilityType::None)
	{
		PlayerCharacter->SetAbility(SelectedAbility);
	}

	CloseWheel();
}

void UKzAbilityWheelViewModel::SetSelectedAbility(const EAbilityType AbilityType)
{
	if (AbilitySlotArray.Contains(AbilityType))
	{
		UE_MVVM_SET_PROPERTY_VALUE(SelectedAbility, AbilityType);
		UpdateSelectionAngle();
	}
}

void UKzAbilityWheelViewModel::AddSelectionDirection(const FVector2D DirectionDelta)
{
	if (!bIsOpen || DirectionDelta.IsNearlyZero() || AbilitySlotArray.IsEmpty())
	{
		return;
	}

	AccumulatedSelectionDirection += DirectionDelta;

	const FVector2D CorrectedDirection = FVector2D(AccumulatedSelectionDirection.X, -AccumulatedSelectionDirection.Y);
	const float AngleFromTop = FMath::Fmod(FMath::Atan2(CorrectedDirection.X, CorrectedDirection.Y) + 2.0f * PI, 2.0f * PI);
	const int32 SlotIndex = FMath::RoundToInt(AngleFromTop / (2.0f * PI / AbilitySlotArray.Num())) % AbilitySlotArray.Num();

	UE_MVVM_SET_PROPERTY_VALUE(SelectedAbility, AbilitySlotArray[SlotIndex]);

	UpdateSelectionAngle();
}

void UKzAbilityWheelViewModel::UpdateSelectionAngle()
{
	const int32 SlotIndex = AbilitySlotArray.IndexOfByKey(SelectedAbility);
	const float NewSelectionAngle = SlotIndex != INDEX_NONE ? 360.0f * SlotIndex / AbilitySlotArray.Num() : 0.0f;

	UE_MVVM_SET_PROPERTY_VALUE(SelectionAngle, NewSelectionAngle);
}
