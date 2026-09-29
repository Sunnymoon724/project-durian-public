#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RemoteBomb.generated.h"

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
	UPROPERTY(EditDefaultsOnly, Category = "Shape")
	ERemoteBombShape BombShape = ERemoteBombShape::None;

	/** Socket used while a player is carrying this bomb. */
	UPROPERTY(EditDefaultsOnly, Category = "Carry")
	FName HeldSocketName = TEXT("hand_r");

	/** Fine-tuning per bomb Blueprint; defaults keep the bomb just outside the palm. */
	UPROPERTY(EditDefaultsOnly, Category = "Carry")
	FVector HeldSocketOffset = FVector(8.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Carry")
	FRotator HeldSocketRotation = FRotator::ZeroRotator;

	bool bHeld = false;
	
	bool IsValid() const;
};
