#pragma once

#include "CoreMinimal.h"
#include "UIEnums.generated.h"

UENUM(BlueprintType)
enum class EUIPriorityType : uint8
{
	Backmost,
	Back,
	Middle,
	Fore,
	Foremost
};
