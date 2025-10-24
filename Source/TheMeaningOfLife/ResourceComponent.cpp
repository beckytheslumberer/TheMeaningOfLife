#include "ResourceComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UResourceComponent::UResourceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Energy = 250.0f;
	MaxEnergy = 1000.0f;
	Water = 250.0f;
	MaxWater = 1000.0f;
	LifeEssence = 25;
	MaxLifeEssence = 100;
	OrganismCap = 32;
	PlantCap = 16;

	PlayerMetabolismRate = 1.0f;
}


// Called when the game starts
void UResourceComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UResourceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Player metabolism - lose energy
	Energy -= PlayerMetabolismRate * DeltaTime;
	Energy = FMath::Max(Energy, 0.0f);

	// Check for death
	if (Energy <= 0.0f)
	{
		OnPlayerDeath();
	}
}

void UResourceComponent::AddEnergy(float Amount)
{
	Energy += Amount;
	Energy = FMath::Min(Energy, MaxEnergy);
}

void UResourceComponent::AddWater(float Amount)
{
	Water += Amount;
	Water = FMath::Min(Water, MaxWater);
}

void UResourceComponent::OnPlayerDeath()
{
	UE_LOG(LogTemp, Error, TEXT("PLAYER HAS DIED - GAME OVER"));

	// Pause game
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGamePaused(World, true);
	}

	// TODO: Show death screen UI
}

bool UResourceComponent::SpendResources(float WaterCost, float EnergyCost, int32 LifeEssenceCost)
{
	// Check if we have enough
	if (Energy < EnergyCost || Water < WaterCost || LifeEssence < LifeEssenceCost)
	{
		return false;
	}

	// Spend it
	Energy -= EnergyCost;
	Water -= WaterCost;
	LifeEssence -= LifeEssenceCost;

	return true;
}

bool UResourceComponent::CanSpawnOrganism(int32 OrganismCount)
{
	// Check if we have room to spawn another organism
	if (OrganismCount < OrganismCap)
	{
		return true;
	}

	// Otherwise return false
	return false;
}

bool UResourceComponent::CanSpawnPlant(int32 PlantCount)
{
	// Check if we have room to spawn another plant
	if (PlantCount < PlantCap)
	{
		return true;
	}

	// Otherwise return false
	return false;
}