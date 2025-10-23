#include "EnvironmentManager.h"
#include "FoodActor.h"
#include "OrganismActor.h"
#include "PlantActor.h"
#include "DrawDebugHelpers.h"

// Sets default values
AEnvironmentManager::AEnvironmentManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default grid settings
	GridWidth = 20;
	GridHeight = 20;
	CellSize = 200.0f;

	// Plant settings
	InitialPlantCount = 10;
	PlantSpawnOffset = 50.0f;

	// Organism settings
	InitialOrganismCount = 30;
	OrganismSpawnOffset = 50.0f;

	// Visualization
	bShowGridLines = true;
}

// Called when the game starts or when spawned
void AEnvironmentManager::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("Environment Manager initialized: %dx%d grid, cell size %f"),
		GridWidth, GridHeight, CellSize);

	SpawnInitialPlants();
	SpawnInitialOrganism();
}

// Called every frame
void AEnvironmentManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Draw grid for visualization
	if (bShowGridLines)
	{
		DrawGrid();
	}
}

void AEnvironmentManager::SpawnInitialPlants()
{
	if (!PlantActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("PlantActorClass not set in EnvironmentManager!"));
		return;
	}

	for (int32 i = 0; i < InitialPlantCount; i++)
	{
		SpawnPlantAtRandomCell();
	}
}

void AEnvironmentManager::SpawnPlantAtRandomCell()
{
	if (!PlantActorClass)
		return;

	int32 RandomX = FMath::RandRange(1, GridWidth - 2); // No plants on edge (0 or -1) or they will spawn food outside of the playable bounds
	int32 RandomY = FMath::RandRange(1, GridHeight - 2); // No plants on edge (0 or -1) or they will spawn food outside of the playable bounds

	FVector SpawnLocation = GetWorldPositionFromGridCell(RandomX, RandomY);
	SpawnLocation.Z = PlantSpawnOffset;

	FActorSpawnParameters SpawnParams;
	AActor* NewPlant = GetWorld()->SpawnActor<APlantActor>(PlantActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (NewPlant)
	{
		// Give the plant a reference to the food class
		APlantActor* Plant = Cast<APlantActor>(NewPlant);
		if (Plant && FoodActorClass)
		{
			Plant->FoodActorClass = FoodActorClass;
		}

		SpawnedPlants.Add(NewPlant);
		// UE_LOG(LogTemp, Log, TEXT("Spawned plant at grid cell (%d, %d)"), RandomX, RandomY);
	}
}

void AEnvironmentManager::SpawnInitialOrganism()
{
	if (!OrganismActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("OrganismActorClass not set in EnvironmentManager!"));
		return;
	}

	for (int32 i = 0; i < InitialOrganismCount; i++)
	{
		SpawnOrganismAtRandomCell();
	}
}

void AEnvironmentManager::SpawnOrganismAtRandomCell()
{
	if (!OrganismActorClass)
		return;

	// Pick a random grid cell
	int32 RandomX = FMath::RandRange(0, GridWidth - 1);
	int32 RandomY = FMath::RandRange(0, GridHeight - 1);

	FVector SpawnLocation = GetWorldPositionFromGridCell(RandomX, RandomY);
	SpawnLocation.Z = OrganismSpawnOffset; // Spawn slightly above ground

	FActorSpawnParameters SpawnParams;
	AActor* NewOrganism = GetWorld()->SpawnActor<AOrganismActor>(OrganismActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (NewOrganism)
	{
		SpawnedOrganisms.Add(NewOrganism);
		// UE_LOG(LogTemp, Log, TEXT("Spawned organism at grid cell (%d, %d)"), RandomX, RandomY);
	}
}

FVector AEnvironmentManager::GetWorldPositionFromGridCell(int32 X, int32 Y)
{
	// Center the grid around the manager's location
	FVector ManagerLocation = GetActorLocation();

	float WorldX = ManagerLocation.X + (X - GridWidth / 2.0f) * CellSize;
	float WorldY = ManagerLocation.Y + (Y - GridHeight / 2.0f) * CellSize;

	return FVector(WorldX, WorldY, ManagerLocation.Z);
}

bool AEnvironmentManager::IsWithinBounds(FVector Location)
{
	FVector ManagerLocation = GetActorLocation();

	float HalfGridWorldWidth = (GridWidth * CellSize) / 2.0f;
	float HalfGridWorldHeight = (GridHeight * CellSize) / 2.0f;

	return (Location.X >= ManagerLocation.X - HalfGridWorldWidth &&
		Location.X <= ManagerLocation.X + HalfGridWorldWidth &&
		Location.Y >= ManagerLocation.Y - HalfGridWorldHeight &&
		Location.Y <= ManagerLocation.Y + HalfGridWorldHeight);
}

void AEnvironmentManager::DrawGrid()
{
	FVector ManagerLocation = GetActorLocation();

	// Draw vertical lines
	for (int32 X = 0; X <= GridWidth; X++)
	{
		FVector Start = GetWorldPositionFromGridCell(X, 0);
		FVector End = GetWorldPositionFromGridCell(X, GridHeight);

		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, -1.0f, 0, 2.0f);
	}

	// Draw horizontal lines
	for (int32 Y = 0; Y <= GridHeight; Y++)
	{
		FVector Start = GetWorldPositionFromGridCell(0, Y);
		FVector End = GetWorldPositionFromGridCell(GridWidth, Y);

		DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, -1.0f, 0, 2.0f);
	}
}