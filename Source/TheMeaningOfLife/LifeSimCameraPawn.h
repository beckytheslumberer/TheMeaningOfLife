#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LifeSimCameraPawn.generated.h"

UCLASS()
class THEMEANINGOFLIFE_API ALifeSimCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALifeSimCameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComponent;
};