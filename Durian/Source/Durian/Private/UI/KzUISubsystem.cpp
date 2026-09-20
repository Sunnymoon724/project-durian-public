#include "UI/KzUISubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "View/MVVMView.h"
#include "MVVMViewModelBase.h"

APlayerController* UKzUISubsystem::GetPlayerController() const
{
	const UGameInstance* GameInstance = GetGameInstance();

	return GameInstance ? GameInstance->GetFirstLocalPlayerController() : nullptr;
}

UUserWidget* UKzUISubsystem::GetOrCreateUIRoot()
{
	if (IsValid(UIRootWidget))
	{
		return UIRootWidget;
	}

	const TSubclassOf<UUserWidget> RootClass = UIRootWidgetClass.LoadSynchronous();

	APlayerController* PlayerController = GetPlayerController();

	if (!RootClass || !PlayerController)
	{
		return nullptr;
	}

	UIRootWidget = CreateWidget<UUserWidget>(PlayerController, RootClass);

	if (IsValid(UIRootWidget))
	{
		UIRootWidget->AddToPlayerScreen();
	}

	return UIRootWidget;
}

UKzBaseWidget* UKzUISubsystem::FindWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UClass* WidgetClass)
{
	const TObjectPtr<UUIWidgetCollection>* Collection = WidgetMultiMap.Find(WidgetClass);

	if (Collection == nullptr || !IsValid(*Collection))
	{
		return nullptr;
	}

	for (UKzBaseWidget* Widget : (*Collection)->WidgetArray)
	{
		if (IsValid(Widget))
		{
			return Widget;
		}
	}

	return nullptr;
}

void UKzUISubsystem::AddWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UKzBaseWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return;
	}
	
	TObjectPtr<UUIWidgetCollection>& Collection = WidgetMultiMap.FindOrAdd(Widget->GetClass());

	if (!Collection)
	{
		Collection = NewObject<UUIWidgetCollection>(this);
	}

	Collection->WidgetArray.AddUnique(Widget);
}

void UKzUISubsystem::RemoveWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UKzBaseWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return;
	}

	const TObjectPtr<UUIWidgetCollection>* Collection = WidgetMultiMap.Find(Widget->GetClass());

	if (Collection == nullptr || !IsValid(*Collection))
	{
		return;
	}

	(*Collection)->WidgetArray.Remove(Widget);

	if ((*Collection)->WidgetArray.IsEmpty())
	{
		WidgetMultiMap.Remove(Widget->GetClass());
	}
}

void UKzUISubsystem::BindViewModel(const UKzBaseWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return;
	}

	UMVVMView* View = Widget->GetExtension<UMVVMView>();

	if (!View)
	{
		return;
	}

	for (const auto& [ContextName, ViewModelClass] : Widget->GetViewModelBindingArray())
	{
		if (ContextName.IsNone() || !ViewModelClass)
		{
			continue;
		}

		TObjectPtr<UMVVMViewModelBase>& ViewModel = ViewModelRegistry.FindOrAdd(ViewModelClass);

		if (!ViewModel)
		{
			ViewModel = NewObject<UMVVMViewModelBase>(this, ViewModelClass);
		}

		View->SetViewModel(ContextName, TScriptInterface<INotifyFieldValueChanged>(ViewModel));
	}
}

int32 UKzUISubsystem::GetZOrder(const UKzBaseWidget* Widget)
{
	return Widget ? static_cast<int32>(Widget->GetPriorityType()) * 100 : 200;
}

void UKzUISubsystem::Register(const TSubclassOf<UKzBaseWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return;
	}

	if (FindWidget(OpenWidgetMultiMap, WidgetClass.Get()))
	{
		return;
	}

	if (FindWidget(PoolWidgetMultiMap, WidgetClass.Get()))
	{
		return;
	}

	APlayerController* PlayerController = GetPlayerController();

	if (!PlayerController)
	{
		return;
	}

	UKzBaseWidget* Widget = CreateWidget<UKzBaseWidget>(PlayerController, WidgetClass);

	if (!IsValid(Widget))
	{
		return;
	}

	BindViewModel(Widget);

	Widget->SetVisibility(ESlateVisibility::Collapsed);

	AddWidget(PoolWidgetMultiMap, Widget);
}

UKzBaseWidget* UKzUISubsystem::Open(const TSubclassOf<UKzBaseWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}
	
	UKzBaseWidget* Widget = FindWidget(OpenWidgetMultiMap, WidgetClass.Get());

	if (!Widget)
	{
		Widget = FindWidget(PoolWidgetMultiMap, WidgetClass.Get());

		if (Widget)
		{
			RemoveWidget(PoolWidgetMultiMap, Widget);
		}
		else
		{
			APlayerController* PlayerController = GetPlayerController();

			if (PlayerController)
			{
				Widget = CreateWidget<UKzBaseWidget>(PlayerController, WidgetClass);
			}
		}

		if (!IsValid(Widget))
		{
			return nullptr;
		}

		BindViewModel(Widget);

		if (!Widget->IsInViewport())
		{
			Widget->AddToPlayerScreen(GetZOrder(Widget));
		}

		AddWidget(OpenWidgetMultiMap, Widget);
	}

	Widget->SetVisibility(ESlateVisibility::Visible);

	return Widget;
}

UKzBaseWidget* UKzUISubsystem::Get(const TSubclassOf<UKzBaseWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}
	
	UKzBaseWidget* Widget = FindWidget(OpenWidgetMultiMap, WidgetClass.Get());

	if (!Widget)
	{
		if (FindWidget(PoolWidgetMultiMap, WidgetClass.Get()))
		{
			return Open(WidgetClass);
		}

		return nullptr;
	}

	return Widget;
}

void UKzUISubsystem::Close(UKzBaseWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return;
	}

	RemoveWidget(OpenWidgetMultiMap, Widget);

	Widget->SetVisibility(ESlateVisibility::Collapsed);

	AddWidget(PoolWidgetMultiMap, Widget);
}

void UKzUISubsystem::Destroy(UKzBaseWidget* Widget)
{
	if (!IsValid(Widget))
	{
		return;
	}

	RemoveWidget(OpenWidgetMultiMap, Widget);
	RemoveWidget(PoolWidgetMultiMap, Widget);

	Widget->RemoveFromParent();
}

void UKzUISubsystem::ClearAll()
{
	for (const TPair<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& Pair : OpenWidgetMultiMap)
	{
		if (IsValid(Pair.Value))
		{
			for (UKzBaseWidget* Widget : Pair.Value->WidgetArray)
			{
				if (IsValid(Widget))
				{
					Widget->RemoveFromParent();
				}
			}
		}
	}

	for (const TPair<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& Pair : PoolWidgetMultiMap)
	{
		if (IsValid(Pair.Value))
		{
			for (UKzBaseWidget* Widget : Pair.Value->WidgetArray)
			{
				if (IsValid(Widget))
				{
					Widget->RemoveFromParent();
				}
			}
		}
	}

	OpenWidgetMultiMap.Empty();
	PoolWidgetMultiMap.Empty();
	ViewModelRegistry.Empty();

	if (IsValid(UIRootWidget))
	{
		UIRootWidget->RemoveFromParent();
	}

	UIRootWidget = nullptr;
}
