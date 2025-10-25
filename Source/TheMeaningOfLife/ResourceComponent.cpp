#include "ResourceComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OrganismActor.h"

// Sets default values for this component's properties
UResourceComponent::UResourceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Energy = 250.0f;
	MaxEnergy = 1000.0f;
	PlayerMetabolismRate = 1.0f;
	Water = 250.0f;
	MaxWater = 1000.0f;
	LifeEssence = 25;
	MaxLifeEssence = 50;
	OrganismCap = 32;
	PlantCap = 16;

	OrganismMetabolismRate = 0.5f;
	OrganismEnergyContribution = 0.2f;
	OrganismCount = 0;
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

	// Organism metabolism - gain energy
	Energy += OrganismCount * OrganismEnergyContribution * OrganismMetabolismRate * DeltaTime;

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

bool UResourceComponent::SpendResources(float EnergyCost, float WaterCost, int32 LifeEssenceCost)
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

int32 UResourceComponent::GetOrganismCount()
{
	return OrganismCount;
}

int32 UResourceComponent::GetPlantCount()
{
	return PlantCount;
}

int32 UResourceComponent::GetOrganismCap()
{
	return OrganismCap;
}

int32 UResourceComponent::GetPlantCap()
{
	return PlantCap;
}

float UResourceComponent::GetEnergyPercent()
{
	return Energy / MaxEnergy;
}

float UResourceComponent::GetWaterPercent()
{
	return Water / MaxWater;
}

FString UResourceComponent::OrganismInfoToString()
{
	return FString("Organisms: " + FString::FromInt(OrganismCount) + "/" + FString::FromInt(OrganismCap));
}

FString UResourceComponent::PlantInfoToString()
{
	return FString("Plants: " + FString::FromInt(PlantCount) + "/" + FString::FromInt(PlantCap));
}

FString UResourceComponent::LifeEssenceInfoToString()
{
	return FString("Life Essence: " + FString::FromInt(LifeEssence) + "/" + FString::FromInt(MaxLifeEssence));
}

bool UResourceComponent::CanSpawnOrganism()
{
	// Check if we have room to spawn another organism
	if (OrganismCount < OrganismCap)
	{
		return true;
	}

	// Otherwise return false
	return false;
}

bool UResourceComponent::CanSpawnPlant()
{
	// Check if we have room to spawn another plant
	if (PlantCount < PlantCap)
	{
		return true;
	}

	// Otherwise return false
	return false;
}

float UResourceComponent::GetOrganismMetabolismRate()
{
	return OrganismMetabolismRate;
}

bool UResourceComponent::AddOrganism()
{
	if (OrganismCount < OrganismCap)
	{
		OrganismCount += 1;
		return true;
	}

	return false;
}

bool UResourceComponent::RemoveOrganism()
{
	if (OrganismCount > 0)
	{
		OrganismCount -= 1;
		return true;
	}

	return false;
}

bool UResourceComponent::AddPlant()
{
	if (PlantCount < PlantCap)
	{
		PlantCount += 1;
		return true;
	}

	return false;
}

bool UResourceComponent::RemovePlant()
{
	if (PlantCount > 0)
	{
		PlantCount -= 1;
		return true;
	}

	return false;
}