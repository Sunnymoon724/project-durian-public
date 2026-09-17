#pragma once

#include "CoreMinimal.h"
#include "Abilities/AbilityModeTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AbilityModeSubsystem.generated.h"

/**
 * 능력 모드의 전역 상태와 월드 객체 알림을 관리한다.
 * 플레이어는 모드 전환만 요청하고, 등록 객체는 자신의 반응을 직접 처리한다.
 */
UCLASS()
class DURIAN_API UAbilityModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** IAbilityModeListener를 구현한 객체를 등록하고, 활성 모드 상태를 즉시 동기화한다. */
	UFUNCTION(BlueprintCallable, Category = "Ability Mode")
	bool RegisterAbilityModeListener(UObject* Listener);

	UFUNCTION(BlueprintCallable, Category = "Ability Mode")
	void UnregisterAbilityModeListener(UObject* Listener);

	/** 지정한 모드를 켜거나 끄고, 해당 모드에 반응하는 등록 객체에 알린다. */
	UFUNCTION(BlueprintCallable, Category = "Ability Mode")
	void SetAbilityModeActive(EAbilityMode Mode, bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Ability Mode")
	bool IsAbilityModeActive(EAbilityMode Mode) const;

	virtual void Deinitialize() override;

private:
	void NotifyListener(UObject* Listener, EAbilityMode Mode, bool bEnabled) const;
	void RemoveInvalidListeners();

	TMap<TWeakObjectPtr<UObject>, TSet<EAbilityMode>> RegisteredListeners;
	TSet<EAbilityMode> ActiveModes;
};
