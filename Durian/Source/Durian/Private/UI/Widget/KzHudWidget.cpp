#include "UI/Widget/KzHudWidget.h"
#include "UI/ViewModel/KzHudViewModel.h"
#include "View/MVVMView.h"

TSubclassOf<UKzBaseWidget> UKzHudWidget::GetWidgetClass()
{
	static const TSoftClassPtr<UKzHudWidget> WidgetClass(FSoftObjectPath(TEXT("/Game/_BP/Widgets/WBP_Hud.WBP_Hud_C")));

	return WidgetClass.LoadSynchronous();
}

void UKzHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UMVVMView* View = GetExtension<UMVVMView>();

	if (!View)
	{
		return;
	}

	for (const FMVVMView_Source& Source : View->GetSources())
	{
		if (UKzHudViewModel* ViewModel = Cast<UKzHudViewModel>(Source.Source))
		{
			ViewModel->Initialize();

			return;
		}
	}
}
