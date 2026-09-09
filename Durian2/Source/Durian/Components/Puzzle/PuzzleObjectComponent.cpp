#include "Components/Puzzle/PuzzleObjectComponent.h"

UPuzzleObjectComponent::UPuzzleObjectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPuzzleObjectComponent::HasObjectType(const EPuzzleObjectType ObjectType) const
{
	const int32 TypeMask = static_cast<int32>(ObjectType);
	return TypeMask != 0 && (ObjectTypes & TypeMask) == TypeMask;
}
