#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/Widget/KzBaseWidget.h"
#include "KzUISubsystem.generated.h"

class APlayerController;
class UUserWidget;

UCLASS()
class DURIAN_API UUIWidgetCollection : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UKzBaseWidget>> WidgetArray;
};

UCLASS(Config=Game)
class DURIAN_API UKzUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	template <typename TWidget>
	TWidget* Open()
	{
		static_assert(TIsDerivedFrom<TWidget, UKzBaseWidget>::Value, "TWidget must derive from UKzBaseWidget");

		return Cast<TWidget>(Open(TWidget::GetWidgetClass()));
	}

	template <typename TWidget>
	TWidget* Get()
	{
		static_assert(TIsDerivedFrom<TWidget, UKzBaseWidget>::Value, "TWidget must derive from UKzBaseWidget");

		return Cast<TWidget>(Get(TWidget::GetWidgetClass()));
	}

	template <typename TWidget>
	void Register()
	{
		static_assert(TIsDerivedFrom<TWidget, UKzBaseWidget>::Value, "TWidget must derive from UKzBaseWidget");

		Register(TWidget::GetWidgetClass());
	}

	void Close(UKzBaseWidget* Widget);
	void Destroy(UKzBaseWidget* Widget);

	void ClearAll();

private:
	void Register(const TSubclassOf<UKzBaseWidget> WidgetClass);
	UKzBaseWidget* Open(const TSubclassOf<UKzBaseWidget> WidgetClass);
	UKzBaseWidget* Get(const TSubclassOf<UKzBaseWidget> WidgetClass);

	UPROPERTY(Config)
	TSoftClassPtr<UUserWidget> UIRootWidgetClass;
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> UIRootWidget;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>> OpenWidgetMultiMap;
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>> PoolWidgetMultiMap;

	APlayerController* GetPlayerController() const;

	UUserWidget* GetOrCreateUIRoot();

	static UKzBaseWidget* FindWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UClass* WidgetClass);

	void AddWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UKzBaseWidget* Widget);
	static void RemoveWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UKzBaseWidget* Widget);

	static int32 GetZOrder(const UKzBaseWidget* Widget);
};
