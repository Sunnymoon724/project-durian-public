#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RemoteBomb.generated.h"

class AKzPlayerCharacter;
class UStaticMeshComponent;
class UNiagaraSystem;

UENUM(BlueprintType)
enum class ERemoteBombShape : uint8
{
	Sphere,
	Cube
};

UCLASS()
class DURIAN_API ARemoteBomb : public AActor
{
	GENERATED_BODY()

public:
	ARemoteBomb();
	void Initialize(ERemoteBombShape InShape, AKzPlayerCharacter* InOwner);
	void Hold(float Height);
	void Place(const FVector& Location, const FVector& Impulse);
	bool IsHeld() const { return bHeld; }
	ERemoteBombShape GetShape() const { return Shape; }
	UStaticMeshComponent* GetBombMesh() const { return BombMesh; }
	void PlayExplosionEffect(float Radius) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Remote Bomb")
	TObjectPtr<UStaticMeshComponent> BombMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Remote Bomb|VFX")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	ERemoteBombShape Shape = ERemoteBombShape::Sphere;
	TWeakObjectPtr<AKzPlayerCharacter> OwningCharacter;
	bool bHeld = false;
};
