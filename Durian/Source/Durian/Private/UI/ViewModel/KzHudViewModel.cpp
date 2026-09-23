#include "UI/ViewModel/KzHudViewModel.h"
#include "Framework/Player/KzPlayerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "UI/KzAbilityUtility.h"

void UKzHudViewModel::Initialize()
{
	Super::Initialize();

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;

	if (PlayerController)
	{
		PlayerController->OnPossessedPawnChanged.AddUniqueDynamic(this, &UKzHudViewModel::HandlePossessedPawnChanged);

		HandlePossessedPawnChanged(nullptr, PlayerController->GetPawn());
	}
}

void UKzHudViewModel::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (PlayerCharacter.IsValid())
	{
		PlayerCharacter->OnAbilityChanged.RemoveAll(this);
	}

	PlayerCharacter = Cast<AKzPlayerCharacter>(NewPawn);

	if (PlayerCharacter.IsValid())
	{
		PlayerCharacter->OnAbilityChanged.AddUObject(this, &UKzHudViewModel::HandleAbilityChanged);
	}

	RefreshCurrentAbilityBrush();
}

void UKzHudViewModel::RefreshCurrentAbilityBrush()
{
	const EAbilityType AbilityType = PlayerCharacter.IsValid() ? PlayerCharacter->GetCurrentAbilityType() : EAbilityType::None;
	const FSlateBrush AbilityBrush = GetAbilityIconBrush(AbilityType);

	UE_MVVM_SET_PROPERTY_VALUE(CurrentAbilityBrush, AbilityBrush);
}

void UKzHudViewModel::HandleAbilityChanged(const EAbilityType AbilityType)
{
	const FSlateBrush AbilityBrush = GetAbilityIconBrush(AbilityType);
	UE_MVVM_SET_PROPERTY_VALUE(CurrentAbilityBrush, AbilityBrush);
}
