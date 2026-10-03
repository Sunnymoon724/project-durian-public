#pragma once

#include "CoreMinimal.h"
#include "World/Trigger/TriggerSource.h"
#include "TriggerSourcePlate.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UPlayerAbilityComponent;
struct FHitResult;

UENUM(BlueprintType)
enum class ETriggerPlateStartMode : uint8
{
	Off,
	Ready,
	Active
};

/** Trigger source activated by an unheld physics object inside the plate volume. */
UCLASS(Blueprintable)
class DURIAN_API ATriggerSourcePlate : public ATriggerSource
{
	GENERATED_BODY()

public:
	ATriggerSourcePlate();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION(BlueprintCallable, Category = "Trigger Plate")
	void HandleBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Trigger Plate")
	void HandleBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	void Initialize();

	void UpdatePlateState();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Trigger Plate", meta = (AllowPrivateAccess = "true"))
	ETriggerPlateStartMode StartMode = ETriggerPlateStartMode::Ready;

	bool bPlateEnabled = false;
	bool bInitialized = false;

	TWeakObjectPtr<UPlayerAbilityComponent> PlayerAbilityComponent;
};
