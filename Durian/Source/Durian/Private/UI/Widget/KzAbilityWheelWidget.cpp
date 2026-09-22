#include "UI/Widget/KzAbilityWheelWidget.h"
#include "UI/ViewModel/KzAbilityWheelViewModel.h"
#include "View/MVVMView.h"

TSubclassOf<UKzBaseWidget> UKzAbilityWheelWidget::GetWidgetClass()
{
	static const TSoftClassPtr<UKzAbilityWheelWidget> WidgetClass(FSoftObjectPath(TEXT("/Game/_BP/Widgets/WBP_AbilityWheel.WBP_AbilityWheel_C")));

	return WidgetClass.LoadSynchronous();
}

void UKzAbilityWheelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UKzAbilityWheelViewModel* ViewModel = GetViewModel())
	{
		ViewModel->Initialize();
	}
}

UKzAbilityWheelViewModel* UKzAbilityWheelWidget::GetViewModel() const
{
	const UMVVMView* View = GetExtension<UMVVMView>();

	if (!View)
	{
		return nullptr;
	}

	for (const FMVVMView_Source& Source : View->GetSources())
	{
		if (UKzAbilityWheelViewModel* ViewModel = Cast<UKzAbilityWheelViewModel>(Source.Source))
		{
			return ViewModel;
		}
	}

	return nullptr;
}
