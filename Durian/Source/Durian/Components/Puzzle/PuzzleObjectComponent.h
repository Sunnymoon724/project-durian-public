#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Global/EPuzzleObjectType.h"
#include "PuzzleObjectComponent.generated.h"

UCLASS(ClassGroup = (Puzzle), meta = (BlueprintSpawnableComponent))
class DURIAN_API UPuzzleObjectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPuzzleObjectComponent();

	/** Puzzle abilities and behaviors supported by the owning Actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle", meta = (Bitmask, BitmaskEnum = "/Script/Durian.EPuzzleObjectType"))
	int32 ObjectTypes = 0;

	/** Returns true when this object supports the requested puzzle type. */
	UFUNCTION(BlueprintPure, Category = "Puzzle")
	bool HasObjectType(EPuzzleObjectType ObjectType) const;
};
