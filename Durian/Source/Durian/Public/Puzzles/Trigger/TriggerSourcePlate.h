#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Puzzles/Trigger/TriggerSource.h"
#include "TriggerSourcePlate.generated.h"

class UBoxComponent;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UPlayerAbilityComponent;
struct FHitResult;

USTRUCT(BlueprintType)
struct FTriggerSourcePlateMaterialTarget
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trigger Plate|Visual", meta = (DisplayName = "Mesh Component"))
	FComponentReference ComponentReference;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trigger Plate|Visual", meta = (DisplayName = "Material Slot Name"))
	FName MaterialSlotName = TEXT("Material_001");
};

/** Trigger source activated by an unheld physics object inside the plate volume. */
UCLASS(Blueprintable)
class DURIAN_API ATriggerSourcePlate : public ATriggerSource
{
	GENERATED_BODY()

public:
	ATriggerSourcePlate();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void Initialize() override;
	virtual void SetCurrentState(ETriggerSourceState NewState) override;
	virtual void OnEnabledChanged(bool bEnabled) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trigger Plate", meta = (DisplayName = "Material Targets"))
	TArray<FTriggerSourcePlateMaterialTarget> MaterialTargetArray;

private:
	UFUNCTION(BlueprintCallable, Category = "Trigger Plate")
	void HandleBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Trigger Plate")
	void HandleBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	void UpdatePlateState();
	void InitializePlateMaterials();
	void RefreshColor();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> MaterialInstances;

	UPROPERTY(Transient)
	TObjectPtr<UBoxComponent> BoxCollision;

	TWeakObjectPtr<UPlayerAbilityComponent> PlayerAbilityComponent;
};
