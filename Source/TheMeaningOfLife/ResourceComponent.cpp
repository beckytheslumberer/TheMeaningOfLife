#include "ResourceComponent.h"

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

	Energy -= MetabolismRate * DeltaTime;
}

bool UResourceComponent::CanSpawnOrganism(float EnergyCost, int32 OrganismCount)
{
	if ((EnergyCost >= Energy) || (LifeEssence < 1) || (OrganismCount >= OrganismCap))
		return false;
	return true;
}

bool UResourceComponent::CanSpawnPlant(float WaterCost, int32 PlantCount)
{
	if ((WaterCost >= Water) || (LifeEssence < 1) || (PlantCount >= PlantCap))
		return false;

	return true;
}