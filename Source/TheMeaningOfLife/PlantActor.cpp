#include "PlantActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "FoodActor.h"
#include "Kismet/GameplayStatics.h"
#include "LifeSimPlayerController.h"
#include "ResourceComponent.h"

// Sets default values
APlantActor::APlantActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    PlantName = TEXT(""); // Empty for now
    bIsSelected = false;

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Set a default cylinder mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CylinderMesh.Object);
		MeshComponent->SetWorldScale3D(FVector(0.3f, 0.3f, 1.5f));
	}

    // Load a basic material and set to green
    static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
    if (Material.Succeeded())
    {
        UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material.Object, this);
        if (DynMaterial)
        {
            // Green color for plants
            DynMaterial->SetVectorParameterValue(FName("Color"), FLinearColor(0.2f, 0.7f, 0.2f, 1.0f));
            MeshComponent->SetMaterial(0, DynMaterial);
        }
    }

    // Production defaults
    FoodSpawnInterval = 15.0f; // Spawn food every 15 seconds
    MaxFoodNearby = 3; // Keep up to 3 food nearby
    FoodSpawnRadius = 150.0f; // Spawn within 150 units
    FoodCheckRadius = 200.0f; // Check for food within 200 units

    Age = 0.0f;
    TimeSinceLastSpawn = 0.0f;
}

// Called when the game starts or when spawned
void APlantActor::BeginPlay()
{
    Super::BeginPlay();

    // UE_LOG(LogTemp, Warning, TEXT("Plant spawned and ready to produce food"));
    AddPlant();
}

// Called every frame
void APlantActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Age += DeltaTime;
    TimeSinceLastSpawn += DeltaTime;

    // Try to spawn food if enough time has passed
    if (TimeSinceLastSpawn >= FoodSpawnInterval)
    {
        // Only spawn if we don't have too much food nearby
        if (CountNearbyFood() < MaxFoodNearby)
        {
            SpawnFood();
        }

        TimeSinceLastSpawn = 0.0f;
    }
}

void APlantActor::AddPlant()
{
    // Add 1 Plant to the ResourceComponent's PlantCount
    if (ALifeSimPlayerController* PC = Cast<ALifeSimPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        if (UResourceComponent* Resources = PC->FindComponentByClass<UResourceComponent>())
        {
            bool bPlantAdded = Resources->AddPlant();
            if (!bPlantAdded)
            {
                UE_LOG(LogTemp, Warning, TEXT("Plant can't be added"));
            }
        }
    }
}

void APlantActor::RemovePlant()
{
    // Remove 1 Plant from the ResourceComponent's PlantCount
    if (ALifeSimPlayerController* PC = Cast<ALifeSimPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        if (UResourceComponent* Resources = PC->FindComponentByClass<UResourceComponent>())
        {
            bool bPlantRemoved = Resources->RemovePlant();
            if (!bPlantRemoved)
            {
                UE_LOG(LogTemp, Warning, TEXT("Plant can't be removed"));
            }
        }
    }
}

void APlantActor::Die()
{
    RemovePlant();
    Destroy();
}

void APlantActor::SpawnFood()
{
    if (!FoodActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Plant has no FoodActorClass set!"));
        return;
    }

    // Random position near the plant
    FVector RandomOffset = FVector(
        FMath::RandRange(-FoodSpawnRadius, FoodSpawnRadius),
        FMath::RandRange(-FoodSpawnRadius, FoodSpawnRadius),
        0.0f
    );

    FVector SpawnLocation = GetActorLocation() + RandomOffset;
    SpawnLocation.Z = 50.0f; // Spawn at consistent height

    FActorSpawnParameters SpawnParams;
    AActor* NewFood = GetWorld()->SpawnActor<AFoodActor>(FoodActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

    if (NewFood)
    {
        SpawnedFood.Add(NewFood);
        // UE_LOG(LogTemp, Log, TEXT("Plant spawned food! Total nearby: %d"), CountNearbyFood());
    }
}

int32 APlantActor::CountNearbyFood()
{
    // Clean up null references
    SpawnedFood.RemoveAll([](AActor* Food) { return Food == nullptr || !IsValid(Food); });

    // Count food within check radius
    int32 Count = 0;
    TArray<AActor*> AllFood;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFoodActor::StaticClass(), AllFood);

    for (AActor* Food : AllFood)
    {
        float Distance = FVector::Dist(GetActorLocation(), Food->GetActorLocation());
        if (Distance <= FoodCheckRadius)
        {
            Count++;
        }
    }

    return Count;
}

void APlantActor::OnSelected()
{
    bIsSelected = true;

    // Optional: Change color or add visual feedback
    if (MeshComponent)
    {
        // Make it slightly brighter when selected
        UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(MeshComponent->GetMaterial(0));
        if (DynMat)
        {
            DynMat->SetVectorParameterValue(FName("Color"), FLinearColor(0.3f, 1.0f, 0.3f, 1.0f));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Plant selected"));
}

void APlantActor::OnDeselected()
{
    bIsSelected = false;

    // Reset to normal color
    if (MeshComponent)
    {
        UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(MeshComponent->GetMaterial(0));
        if (DynMat)
        {
            DynMat->SetVectorParameterValue(FName("Color"), FLinearColor(0.2f, 0.7f, 0.2f, 1.0f));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Plant deselected"));
}

FString APlantActor::GetDisplayName()
{
    if (PlantName.IsEmpty())
    {
        return FString::Printf(TEXT("Plant #%d"), GetUniqueID());
    }
    return PlantName;
}

TArray<TPair<FString, FString>> APlantActor::GetDisplayInfo()
{
    TArray<TPair<FString, FString>> Info;

    Info.Add(TPair<FString, FString>(TEXT("Age"), FString::Printf(TEXT("%.1f seconds"), Age)));
    Info.Add(TPair<FString, FString>(TEXT("Food Spawn Interval"), FString::Printf(TEXT("%.1fs"), FoodSpawnInterval)));
    Info.Add(TPair<FString, FString>(TEXT("Max Food Nearby"), FString::Printf(TEXT("%d"), MaxFoodNearby)));
    Info.Add(TPair<FString, FString>(TEXT("Current Nearby Food"), FString::Printf(TEXT("%d"), CountNearbyFood())));
    Info.Add(TPair<FString, FString>(TEXT("Time Until Next Food"), FString::Printf(TEXT("%.1fs"), FoodSpawnInterval - TimeSinceLastSpawn)));

    return Info;
}