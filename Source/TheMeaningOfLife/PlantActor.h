#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Selectable.h"
#include "PlantActor.generated.h"

UCLASS()
class THEMEANINGOFLIFE_API APlantActor : public AActor, public ISelectable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlantActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
    // Selectable interface implementation
    virtual void OnSelected() override;
    virtual void OnDeselected() override;
    virtual FString GetDisplayName() override;
    virtual TArray<TPair<FString, FString>> GetDisplayInfo() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant")
    FString PlantName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant")
    bool bIsSelected;

    virtual void Tick(float DeltaTime) override;

    // Visual representation
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Plant")
    class UStaticMeshComponent* MeshComponent;

    // Food production
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Production")
    TSubclassOf<class AFoodActor> FoodActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Production")
    float FoodSpawnInterval; // How often to spawn food (seconds)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Production")
    int32 MaxFoodNearby; // Max food this plant will maintain nearby

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Production")
    float FoodSpawnRadius; // How far from plant to spawn food

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Production")
    float FoodCheckRadius; // Radius to check for existing food

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant")
    float Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Water")
    float Water;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Water")
    float MaxWater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Water")
    float WaterConsumptionRate; // Water used per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Water")
    float LowWaterThreshold; // Below this = slower food production

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Water")
    float FoodSpawnIntervalWellWatered; // Fast food production when water > threshold

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plant|Water")
    float FoodSpawnIntervalDry; // Slow food production when water < threshold

    UFUNCTION()
    void AddWater(float Amount);

private:
    void SpawnFood();
    int32 CountNearbyFood();
    void UpdatePlantColor(bool bIsLowWater);
    void AddPlant();
    void RemovePlant();
    void Die();

    float TimeSinceLastSpawn;
    TArray<AActor*> SpawnedFood;
};