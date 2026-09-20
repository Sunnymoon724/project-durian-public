#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KzScene.generated.h"

class UKzSceneDefinition;

UCLASS(Abstract, Blueprintable)
class DURIAN_API UKzScene : public UObject
{
	GENERATED_BODY()

public:
	virtual bool Initialize(const UKzSceneDefinition* InDefinition);
	virtual void Deinitialize();

	const UKzSceneDefinition* GetDefinition() const { return Definition; }

protected:
	UPROPERTY(Transient)
	TObjectPtr<const UKzSceneDefinition> Definition;
};
