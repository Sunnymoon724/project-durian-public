#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/KzBaseWidget.h"
#include "KzUISubsystem.generated.h"

class APlayerController;
class UUserWidget;
class UMVVMViewModelBase;

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
	void Register(const TSubclassOf<UKzBaseWidget> WidgetClass);
	UKzBaseWidget* Open(const TSubclassOf<UKzBaseWidget> WidgetClass);

	UKzBaseWidget* Get(const TSubclassOf<UKzBaseWidget> WidgetClass);

	void Close(UKzBaseWidget* Widget);
	void Destroy(UKzBaseWidget* Widget);
	void ClearAll();

private:
	UPROPERTY(Config)
	TSoftClassPtr<UUserWidget> UIRootWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> UIRootWidget;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>> OpenWidgetMultiMap;
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>> PoolWidgetMultiMap;

	UPROPERTY(Transient)
	TMap<TSubclassOf<UMVVMViewModelBase>, TObjectPtr<UMVVMViewModelBase>> ViewModelRegistry;

	APlayerController* GetPlayerController() const;

	UUserWidget* GetOrCreateUIRoot();

	static UKzBaseWidget* FindWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UClass* WidgetClass);

	void AddWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UKzBaseWidget* Widget);
	static void RemoveWidget(TMap<TObjectPtr<UClass>, TObjectPtr<UUIWidgetCollection>>& WidgetMultiMap, UKzBaseWidget* Widget);

	void BindViewModel(const UKzBaseWidget* Widget);

	static int32 GetZOrder(const UKzBaseWidget* Widget);
};
