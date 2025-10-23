#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Selectable.h"
#include "OrganismActor.generated.h"

// Struct to store memories of food locations
USTRUCT(BlueprintType)
struct FFoodMemory
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	float TimeSinceFound;

	UPROPERTY()
	bool bStillExists;

	FFoodMemory()
		: Location(FVector::ZeroVector)
		, TimeSinceFound(0.0f)
		, bStillExists(true)
	{
	}

	FFoodMemory(FVector InLocation)
		: Location(InLocation)
		, TimeSinceFound(0.0f)
		, bStillExists(true)
	{
	}
};

UCLASS()
class THEMEANINGOFLIFE_API AOrganismActor : public AActor, public ISelectable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOrganismActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Selectable interface implementation
	virtual void OnSelected() override;
	virtual void OnDeselected() override;
	virtual FString GetDisplayName() override;
	virtual TArray<TPair<FString, FString>> GetDisplayInfo() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	FString OrganismName; // We'll add naming later, for now it can be empty

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	bool bIsSelected;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Core properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	float Energy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	float MaxEnergy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	float MetabolismRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	float Age;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	float DetectionRadius; // How far the organism can "see" food

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism")
	float HungerThreshold; // When energy drops below this, seek food

	// Reproduction properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism|Reproduction")
	float ReproductionThreshold; // Energy level needed to reproduce

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism|Reproduction")
	float ReproductionCost; // Energy spent to create offspring

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism|Reproduction")
	float ReproductionCooldown; // Time between reproductions

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism|Reproduction")
	float ReproductionSpawnOffset; // Time between reproductions

	// Visual representation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organism")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Organism")
	class UWidgetComponent* EnergyBarWidget;

	// Memory properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism|Memory")
	int32 MaxFoodMemories; // How many locations to remember

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Organism|Memory")
	float MemoryDecayTime; // How long before forgetting a location

private:
	void Die();
	void MoveRandomly(float DeltaTime);
	void SeekFood(float DeltaTime);
	bool TryEatNearbyFood();
	void UpdateEnergyBar();
	bool CheckAndHandleBoundaries();
	void TryReproduce();
	void UpdateFoodMemories(float DeltaTime);
	void RememberFoodLocation(FVector Location);
	AActor* FindFoodFromMemory();

	// Movement state
	FVector CurrentMovementDirection;
	float TimeSinceDirectionChange;
	float DirectionChangeInterval;
	float DirectionChangeIntervalMin;
	float DirectionChangeIntervalMax;

	// Reproduction state
	float TimeSinceLastReproduction;

	// Memory state
	TArray<FFoodMemory> FoodMemories;
};
