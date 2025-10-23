#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentManager.generated.h"

UCLASS()
class THEMEANINGOFLIFE_API AEnvironmentManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnvironmentManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// Grid settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	int32 GridWidth; // Number of cells wide

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	int32 GridHeight; // Number of cells tall

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float CellSize; // Size of each grid cell

	// Food spawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	TSubclassOf<class AFoodActor> FoodActorClass;

	// Plant spawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	TSubclassOf<class APlantActor> PlantActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	int32 InitialPlantCount; // How many plants to spawn at start

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float PlantSpawnOffset; // How far above the ground to spawn new plants

	// Organism spawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	TSubclassOf<class AOrganismActor> OrganismActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	int32 InitialOrganismCount; // How many organisms to spawn at start

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float OrganismSpawnOffset; // How far above the ground to spawn new organisms

	// Visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	bool bShowGridLines; // Toggle grid visualization

private:
	void SpawnInitialPlants();
	void SpawnPlantAtRandomCell();

	void SpawnInitialOrganism();
	void SpawnOrganismAtRandomCell();
	FVector GetWorldPositionFromGridCell(int32 X, int32 Y);
	bool IsWithinBounds(FVector Location);
	void DrawGrid();

	TArray<AActor*> SpawnedOrganisms;
	TArray<AActor*> SpawnedPlants;
};
