#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RemoteBomb.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum class ERemoteBombShape : uint8
{
	None,
	Sphere,
	Cube
};

UCLASS()
class DURIAN_API ARemoteBomb : public AActor
{
	GENERATED_BODY()

public:
	ARemoteBomb();
	void Hold(float Height);
	void Place(const FVector& Location, const FVector& Impulse);
	bool IsHeld() const { return bHeld; }
	ERemoteBombShape GetShape() const { return BombShape; }
	void PlayExplosionEffect(float Radius) const;
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSoftObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Shape")
	ERemoteBombShape BombShape = ERemoteBombShape::None;

	bool bHeld = false;
	
	bool IsValid() const;
};
