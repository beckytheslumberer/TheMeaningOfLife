#include "LifeSimCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ALifeSimCameraPawn::ALifeSimCameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Root component
    RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    // Camera component
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(RootScene);
    CameraComponent->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f)); // Angled down
    CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

}

// Called when the game starts or when spawned
void ALifeSimCameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}