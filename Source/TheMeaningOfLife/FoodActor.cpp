#include "FoodActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"

AFoodActor::AFoodActor()
{
 	PrimaryActorTick.bCanEverTick = true;

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Set a default sphere mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
		MeshComponent->SetWorldScale3D(FVector(0.3f, 0.3f, 0.3f)); // Make it smaller
	}

	// Load a basic material from the engine
	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (Material.Succeeded())
	{
		UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(Material.Object, this);
		if (DynMaterial)
		{
			// Set color to magenta
			DynMaterial->SetVectorParameterValue(FName("Color"), FLinearColor(0.69f, 0.15f, 0.55f, 1.0f));
			MeshComponent->SetMaterial(0, DynMaterial);
		}
	}

	// Default energy value
	EnergyValue = 40.0f;
}

// Called when the game starts or when spawned
void AFoodActor::BeginPlay()
{
	Super::BeginPlay();
	
	// UE_LOG(LogTemp, Warning, TEXT("Food spawned with %f energy value"), EnergyValue);
}

void AFoodActor::Consume()
{
	// UE_LOG(LogTemp, Warning, TEXT("Food consumed!"));
	Destroy();
}

