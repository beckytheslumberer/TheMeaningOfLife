#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEMEANINGOFLIFE_API UResourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UResourceComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float Energy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float MaxEnergy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float PlayerMetabolismRate; // Energy lost per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float Water;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float MaxWater;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 LifeEssence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MaxLifeEssence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float OrganismEnergyContribution; // How much each organism gives per their metabolism

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float PlantWaterContribution; // How much each plant gives per their consumption

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	int32 OrganismCount;
	int32 OrganismCap;
	float OrganismMetabolismRate;
	int32 PlantCount;
	int32 PlantCap;
	float PlantConsumptionRate;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void OnPlayerDeath();
	void AddEnergy(float Amount);
	void AddWater(float Amount);
	bool SpendResources(float EnergyCost, float WaterCost, int32 LifeEssenceCost);
	bool CanSpawnOrganism();
	bool CanSpawnPlant();
	bool AddOrganism();
	bool RemoveOrganism();
	bool AddPlant();
	bool RemovePlant();
	float GetOrganismMetabolismRate();
	int32 GetOrganismCount();
	int32 GetOrganismCap();
	int32 GetPlantCount();
	int32 GetPlantCap();
	FString OrganismInfoToString();
	FString PlantInfoToString();
	FString LifeEssenceInfoToString();
	float GetEnergyPercent();
	float GetWaterPercent();
};
