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
	int32 MetabolismRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float Water;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float MaxEnergy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float MaxWater;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 LifeEssence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 MaxLifeEssence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 OrganismCap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	int32 PlantCap;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool CanSpawnOrganism(float EnergyCost, int32 OrganismCount);
	bool CanSpawnPlant(float WaterCost, int32 PlantCount);
};
