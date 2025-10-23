#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoodActor.generated.h"

UCLASS()
class THEMEANINGOFLIFE_API AFoodActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AFoodActor();

protected:
	virtual void BeginPlay() override;

public:
	// How much energy this food provides
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Food")
	float EnergyValue;

	// Visual representation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Food")
	class UStaticMeshComponent* MeshComponent;

	// Called when an organism consumes this food
	void Consume();
};
