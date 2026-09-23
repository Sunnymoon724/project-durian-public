#pragma once

#include "CoreMinimal.h"
#include "Abilities/Core/AbilityModeTypes.h"
#include "UObject/Interface.h"
#include "AbilityModeListener.generated.h"

UINTERFACE(BlueprintType)
class DURIAN_API UAbilityModeListener : public UInterface
{
	GENERATED_BODY()
};

/** 능력 모드 전환을 수신하는 월드 객체의 공통 계약이다. */
class DURIAN_API IAbilityModeListener
{
	GENERATED_BODY()

public:
	/** 이 객체가 반응할 수 있는 능력 모드를 반환한다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability Mode")
	TArray<EAbilityVisualMode> GetSupportedAbilityModes() const;

	/** 등록된 능력 모드가 전환될 때 호출된다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability Mode")
	void OnAbilityModeChanged(EAbilityVisualMode Mode, bool bEnabled);
};
