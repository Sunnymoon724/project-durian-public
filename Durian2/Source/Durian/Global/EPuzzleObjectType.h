#pragma once

#include "CoreMinimal.h"

#include "EPuzzleObjectType.generated.h"

UENUM(BlueprintType, meta = (Bitflags))
enum class EPuzzleObjectType : uint8
{
	None              = 0       UMETA(Hidden),
	Magnetic          = 1 << 0,
	IceSurface        = 1 << 1,
	StasisTarget      = 1 << 2,
	BombReactive      = 1 << 3,
	Destructible      = 1 << 4,
	PressureActivator = 1 << 5,
	Resettable        = 1 << 6
};

ENUM_CLASS_FLAGS(EPuzzleObjectType)
