#pragma once

#include "CoreMinimal.h"
#include "World/Trigger/TriggerSource.h"
#include "TriggerSourcePlate.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
struct FHitResult;

UENUM(BlueprintType)
enum class ETriggerPlateStartMode : uint8
{
	Off,
	Ready,
	Active
};

/** Trigger source activated only by its configured block resting in the plate volume. */
UCLASS(Blueprintable)
class DURIAN_API ATriggerSourcePlate : public ATriggerSource
{
	GENERATED_BODY()

public:
	ATriggerSourcePlate();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	void HandleRequiredBlockHeldChanged(bool bIsHeld);
	void Initialize();
	bool IsRequiredBlockOverlapping() const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Trigger Plate", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> RequiredBlock;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Trigger Plate", meta = (AllowPrivateAccess = "true"))
	ETriggerPlateStartMode StartMode = ETriggerPlateStartMode::Ready;

	bool bPlateEnabled = false;
	bool bInitialized = false;
	FDelegateHandle MagnesisHeldChangedHandle;
};
