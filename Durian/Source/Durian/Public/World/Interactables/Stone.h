#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Stone.generated.h"

class AController;
struct FDamageEvent;

UCLASS()
class DURIAN_API AStone : public AActor
{
	GENERATED_BODY()

public:
	AStone();
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	bool bIsBroken = false;
};
