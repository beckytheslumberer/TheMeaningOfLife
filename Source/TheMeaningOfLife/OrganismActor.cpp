#include "OrganismActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ProgressBar.h"
#include "Blueprint/UserWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "FoodActor.h"
#include "EnvironmentManager.h"
#include "Kismet/GameplayStatics.h"

AOrganismActor::AOrganismActor()
{
 	PrimaryActorTick.bCanEverTick = true;
    
    OrganismName = TEXT(""); // Empty for now
    bIsSelected = false;

	// Create a simple mesh component for visualization
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Set a default cube mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
		MeshComponent->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f)); // Make it a reasonable size
	}

	// Load a basic material from the engine
	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (Material.Succeeded())
	{
		UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material.Object, this);
		if (DynMaterial)
		{
			// Set color to a nice blue/purple for organisms
			DynMaterial->SetVectorParameterValue(FName("Color"), FLinearColor(0.4f, 0.3f, 0.8f, 1.0f));
			MeshComponent->SetMaterial(0, DynMaterial);
		}
	}

	// Create energy bar widget
	EnergyBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnergyBarWidget"));

	// Load the energy bar widget class
	if (EnergyBarWidget)
	{
		// Try to load the widget blueprint
		static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass(TEXT("/Game/UI/WBP_EnergyBar"));
		if (WidgetClass.Succeeded())
		{
			EnergyBarWidget->SetWidgetClass(WidgetClass.Class);
			EnergyBarWidget->SetVisibility(false); // Hidden by default
			EnergyBarWidget->SetupAttachment(RootComponent);
			EnergyBarWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // 100 units above organism
			EnergyBarWidget->SetWidgetSpace(EWidgetSpace::Screen); // Always face camera
			EnergyBarWidget->SetDrawSize(FVector2D(100.0f, 10.0f));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load WBP_EnergyBar!"));
		}
	}

	// Core initializations
	MaxEnergy = 100.0f;
	Energy = 100.0f;
	MetabolismRate = 0.1f; // Loses 0.1 energy per second
	Age = 0.0f;
	MovementSpeed = 100.0f;
	DetectionRadius = 250.0f;
	HungerThreshold = 40.0f;

	// Reproduction initializations
	ReproductionThreshold = 100.0f; // Need 200 energy to reproduce
	ReproductionCost = 60.0f; // Costs 130 energy to make a baby
	ReproductionCooldown = 120.0f; // Wait 2 minutes
	ReproductionSpawnOffset = 25.0f; // Spawn offset 25 cms
	TimeSinceLastReproduction = ReproductionCooldown;

	// Memory initialization - Add these
	MaxFoodMemories = 3; // Remember up to 3 food locations
	MemoryDecayTime = 300.0f; // Forget after 5 minutes

	// Movement initialization
	CurrentMovementDirection = FVector::ZeroVector;
	TimeSinceDirectionChange = 0.0f;
	DirectionChangeIntervalMin = 2.0f; // Change direction only after every 2 seconds
	DirectionChangeIntervalMax = 5.0f; // Change direction at least every 5 seconds
	DirectionChangeInterval = 0.0f;
}

// Called when the game starts or when spawned
void AOrganismActor::BeginPlay()
{
	Super::BeginPlay();
	
	// UE_LOG(LogTemp, Warning, TEXT("Organism spawned with %f energy"), Energy);
}

// Called every frame
void AOrganismActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Age += DeltaTime;
	TimeSinceLastReproduction += DeltaTime;

	// Consume energy over time (metabolism)
	Energy -= MetabolismRate * DeltaTime;

	UpdateEnergyBar();
	UpdateFoodMemories(DeltaTime);

	// Check if organism dies
	if (Energy <= 0.0f)
	{
		Die();
		return;
	}

	// Try to eat nearby food first
	if (TryEatNearbyFood())
	{
		return; // Eating takes this frame
	}

	// Try to reproduce if conditions are met
	TryReproduce();

	// Check boundaries before moving
	CheckAndHandleBoundaries();

	// If hungry, seek food. Otherwise wander randomly
	if (Energy < HungerThreshold)
	{
		SeekFood(DeltaTime);
	}
	else
	{
		MoveRandomly(DeltaTime);
	}
}

void AOrganismActor::Die()
{
	// UE_LOG(LogTemp, Warning, TEXT("Organism died at age %f"), Age);
	Destroy();
}

void AOrganismActor::MoveRandomly(float DeltaTime)
{
	TimeSinceDirectionChange += DeltaTime;

	// Pick a new direction if enough time has passed or we don't have one yet
	if (TimeSinceDirectionChange >= DirectionChangeInterval || CurrentMovementDirection.IsZero())
	{
		CurrentMovementDirection = FVector(
			FMath::RandRange(-1.0f, 1.0f),
			FMath::RandRange(-1.0f, 1.0f),
			0.0f // Keep movement on horizontal plane
		).GetSafeNormal();

		TimeSinceDirectionChange = 0.0f;

		// Pick a random length between interval min and max
		DirectionChangeInterval = FMath::RandRange(DirectionChangeIntervalMin, DirectionChangeIntervalMax);
	}

	// Move in the current direction
	FVector NewLocation = GetActorLocation() + (CurrentMovementDirection * MovementSpeed * DeltaTime);
	SetActorLocation(NewLocation);
}

void AOrganismActor::SeekFood(float DeltaTime)
{
	// Draw detection radius
	DrawDebugSphere(GetWorld(), GetActorLocation(), DetectionRadius, 16, FColor::Red, false, -1.0f, 0, 2.0f);

	// First, try to go to a remembered food location
	AActor* RememberedFood = FindFoodFromMemory();
	if (RememberedFood)
	{
		FVector Direction = (RememberedFood->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FVector NewLocation = GetActorLocation() + (Direction * MovementSpeed * DeltaTime);
		SetActorLocation(NewLocation);

		// Draw green line to show we're using memory
		DrawDebugLine(GetWorld(), GetActorLocation(), RememberedFood->GetActorLocation(),
			FColor::Cyan, false, -1.0f, 0, 2.0f);
		return;
	}

	// If no memory, search normally
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFoodActor::StaticClass(), FoundActors);

	if (FoundActors.Num() == 0)
	{
		// No food found, wander randomly
		MoveRandomly(DeltaTime);
		return;
	}

	// Find the closest food within detection radius
	AActor* ClosestFood = nullptr;
	float ClosestDistance = DetectionRadius;

	for (AActor* Food : FoundActors)
	{
		float Distance = FVector::Dist(GetActorLocation(), Food->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestFood = Food;
		}
	}

	if (ClosestFood)
	{
		// Draw a debug line so we can see it seeking
		DrawDebugLine(GetWorld(), GetActorLocation(), ClosestFood->GetActorLocation(),
			FColor::Green, false, -1.0f, 0, 2.0f);

		// Move toward the closest food
		FVector Direction = (ClosestFood->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FVector NewLocation = GetActorLocation() + (Direction * MovementSpeed * DeltaTime);
		SetActorLocation(NewLocation);
	}
	else
	{
		// No food in range, wander
		MoveRandomly(DeltaTime);
	}
}

bool AOrganismActor::TryEatNearbyFood()
{
	// Find all food actors in the world
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFoodActor::StaticClass(), FoundActors);
	
	for (AActor* Actor : FoundActors)
	{
		float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());

		// If food is very close (within 50 units), eat it
		if (Distance < 50.0f)
		{
			AFoodActor* Food = Cast<AFoodActor>(Actor);
			if (Food)
			{
				// Remember this location before eating
				RememberFoodLocation(Food->GetActorLocation());

				Energy = FMath::Min(Energy + Food->EnergyValue, MaxEnergy);
				// UE_LOG(LogTemp, Warning, TEXT("Organism ate food! Energy now: %f"), Energy);
				Food->Consume();
				return true;
			}
		}
	}
	return false;
}

void AOrganismActor::UpdateEnergyBar()
{
	if (EnergyBarWidget && EnergyBarWidget->GetWidget())
	{
		UUserWidget* Widget = EnergyBarWidget->GetWidget();

		// Find the progress bar in the widget
		UProgressBar* ProgressBar = Cast<UProgressBar>(Widget->GetWidgetFromName(TEXT("EnergyProgressBar")));

		if (ProgressBar)
		{
			float Percent = Energy / MaxEnergy;
			ProgressBar->SetPercent(Percent);
		}
	}
}

bool AOrganismActor::CheckAndHandleBoundaries()
{
	// Find the environment manager
	AEnvironmentManager* EnvManager = nullptr;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnvironmentManager::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		EnvManager = Cast<AEnvironmentManager>(FoundActors[0]);
	}

	if (!EnvManager)
		return false;

	FVector Location = GetActorLocation();
	FVector ManagerLocation = EnvManager->GetActorLocation();

	float HalfGridWidth = (EnvManager->GridWidth * EnvManager->CellSize) / 2.0f;
	float HalfGridHeight = (EnvManager->GridHeight * EnvManager->CellSize) / 2.0f;

	bool HitBoundary = false;

	// Check X boundaries
	if (Location.X < ManagerLocation.X - HalfGridWidth)
	{
		Location.X = ManagerLocation.X - HalfGridWidth;
		CurrentMovementDirection.X = FMath::Abs(CurrentMovementDirection.X); // Bounce right
		HitBoundary = true;
	}
	else if (Location.X > ManagerLocation.X + HalfGridWidth)
	{
		Location.X = ManagerLocation.X + HalfGridWidth;
		CurrentMovementDirection.X = -FMath::Abs(CurrentMovementDirection.X); // Bounce left
		HitBoundary = true;
	}

	// Check Y boundaries
	if (Location.Y < ManagerLocation.Y - HalfGridHeight)
	{
		Location.Y = ManagerLocation.Y - HalfGridHeight;
		CurrentMovementDirection.Y = FMath::Abs(CurrentMovementDirection.Y); // Bounce up
		HitBoundary = true;
	}
	else if (Location.Y > ManagerLocation.Y + HalfGridHeight)
	{
		Location.Y = ManagerLocation.Y + HalfGridHeight;
		CurrentMovementDirection.Y = -FMath::Abs(CurrentMovementDirection.Y); // Bounce down
		HitBoundary = true;
	}

	if (HitBoundary)
	{
		SetActorLocation(Location);
		CurrentMovementDirection.Normalize();
	}

	return HitBoundary;
}

void AOrganismActor::TryReproduce()
{
	// Check if we have enough energy and cooldown is done
	if (Energy < ReproductionThreshold || TimeSinceLastReproduction < ReproductionCooldown)
	{
		return;
	}

	// Pay the energy cost
	Energy -= ReproductionCost;
	TimeSinceLastReproduction = 0.0f;

	// Spawn offspring nearby
	FVector OffsetDirection = FVector(
		FMath::RandRange(-1.0f, 1.0f),
		FMath::RandRange(-1.0f, 1.0f),
		0.0f
	).GetSafeNormal();

	FVector SpawnLocation = GetActorLocation() + (OffsetDirection * 100.0f); // 100 units away
	SpawnLocation.Z = ReproductionSpawnOffset; // Spawn slightly above ground
	
	FActorSpawnParameters SpawnParams;
	AOrganismActor* Offspring = GetWorld()->SpawnActor<AOrganismActor>(
		AOrganismActor::StaticClass(),
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Offspring)
	{
		// Baby starts with half energy
		Offspring->Energy = Offspring->MaxEnergy * 0.5f;

		// UE_LOG(LogTemp, Warning, TEXT("Organism reproduced! Parent energy: %f"), Energy);
	}
}

void AOrganismActor::UpdateFoodMemories(float DeltaTime)
{
	// Age all memories
	for (int32 i = FoodMemories.Num() - 1; i >= 0; i--)
	{
		FoodMemories[i].TimeSinceFound += DeltaTime;

		// Forget old memories
		if (FoodMemories[i].TimeSinceFound > MemoryDecayTime)
		{
			FoodMemories.RemoveAt(i);
		}
	}
}

void AOrganismActor::RememberFoodLocation(FVector Location)
{
	// Check if we already remember this location (nearby)
	for (FFoodMemory& Memory : FoodMemories)
	{
		if (FVector::Dist(Memory.Location, Location) < 100.0f)
		{
			// Refresh this memory
			Memory.TimeSinceFound = 0.0f;
			return;
		}
	}

	// Add new memory
	FoodMemories.Add(FFoodMemory(Location));

	// Forget oldest if too many
	if (FoodMemories.Num() > MaxFoodMemories)
	{
		FoodMemories.RemoveAt(0);
	}

	// UE_LOG(LogTemp, Log, TEXT("Organism remembered location! Total memories: %d"), FoodMemories.Num());
}

AActor* AOrganismActor::FindFoodFromMemory()
{
	if (FoodMemories.Num() == 0)
		return nullptr;

	// Find all food in the world
	TArray<AActor*> AllFood;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFoodActor::StaticClass(), AllFood);

	// Check each memory to see if food still exists there
	for (FFoodMemory& Memory : FoodMemories)
	{
		for (AActor* Food : AllFood)
		{
			// If food is near a remembered location, go there!
			if (FVector::Dist(Food->GetActorLocation(), Memory.Location) < 150.0f)
			{
				return Food;
			}
		}
	}

	return nullptr;
}

void AOrganismActor::OnSelected()
{
	bIsSelected = true;

	// Make the energy bar visible
	if (EnergyBarWidget)
	{
		EnergyBarWidget->SetVisibility(true);
	}

	// Optional: Add a visual indicator (we can add an outline or glow later)
	UE_LOG(LogTemp, Log, TEXT("Organism selected"));
}

void AOrganismActor::OnDeselected()
{
	bIsSelected = false;

	// Hide the energy bar
	if (EnergyBarWidget)
	{
		EnergyBarWidget->SetVisibility(false);
	}

	UE_LOG(LogTemp, Log, TEXT("Organism deselected"));
}

FString AOrganismActor::GetDisplayName()
{
	if (OrganismName.IsEmpty())
	{
		return FString::Printf(TEXT("Organism #%d"), GetUniqueID());
	}
	return OrganismName;
}

TArray<TPair<FString, FString>> AOrganismActor::GetDisplayInfo()
{
	TArray<TPair<FString, FString>> Info;

	Info.Add(TPair<FString, FString>(TEXT("Energy"), FString::Printf(TEXT("%.1f / %.1f"), Energy, MaxEnergy)));
	Info.Add(TPair<FString, FString>(TEXT("Age"), FString::Printf(TEXT("%.1f seconds"), Age)));
	Info.Add(TPair<FString, FString>(TEXT("Metabolism"), FString::Printf(TEXT("%.2f/s"), MetabolismRate)));
	Info.Add(TPair<FString, FString>(TEXT("Movement Speed"), FString::Printf(TEXT("%.0f"), MovementSpeed)));
	Info.Add(TPair<FString, FString>(TEXT("Food Memories"), FString::Printf(TEXT("%d"), FoodMemories.Num())));

	return Info;
}