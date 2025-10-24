#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ResourceComponent.h"
#include "LifeSimPlayerController.generated.h"

UCLASS()
class THEMEANINGOFLIFE_API ALifeSimPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
    ALifeSimPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UResourceComponent* MyResourceComponent; // Declare your component

public:
    virtual void Tick(float DeltaTime) override;

    // Camera settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraMoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraZoomSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraRotateSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraAngleSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MinZoomHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MaxZoomHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MinCameraAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MaxCameraAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float DefaultCameraHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float DefaultCameraAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MouseRotateSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float MouseAngleSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float EdgePanBorderSize; // Pixels from edge to trigger pan

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    bool bUseEdgePanning;

    // Simulation speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float CurrentSimulationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float MinSimulationSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simulation")
    float MaxSimulationSpeed;

    UPROPERTY()
    TSubclassOf<UUserWidget> SelectionInfoWidgetClass;

    UPROPERTY()
    TSubclassOf<UUserWidget> InfoRowWidgetClass;

    UPROPERTY()
    TSubclassOf<UUserWidget> SimulationSpeedWidgetClass;

    UPROPERTY()
    TSubclassOf<UUserWidget> ResourceBarWidgetClass;

    UPROPERTY()
    class UUserWidget* SelectionInfoWidget;

    UPROPERTY()
    class UUserWidget* SimulationSpeedWidget;

    UPROPERTY()
    class UUserWidget* ResourceBarWidget;

    // Spawn UI
    UPROPERTY()
    TSubclassOf<UUserWidget> SpawnButtonsWidgetClass;

    UPROPERTY()
    class UUserWidget* SpawnButtonsWidget;

    // Spawn functions
    UFUNCTION()
    void EnterOrganismSpawnMode();
    UFUNCTION()
    void EnterPlantSpawnMode();
    UFUNCTION()
    void ExitSpawnMode();

    // Spawn mode
    UPROPERTY()
    bool bIsInSpawnMode;

    UPROPERTY()
    TSubclassOf<AActor> PendingSpawnClass; // What we're about to spawn

    enum class ESpawnType : uint8
    {
        None,
        Organism,
        Plant
    };

    ESpawnType CurrentSpawnType;

private:
    void HandleSpawnClick(const FVector& MySpawnLocation);
    void CreateSpawnUI();
    void SetupSpawnButtonCallbacks();
    FVector GetMouseWorldPosition();

    class UButton* SpawnOrganismButton;
    class UButton* SpawnPlantButton;

    // Camera movement
    void MoveCameraForward(float Value);
    void MoveCameraRight(float Value);
    void ZoomCamera(float Value);
    void HandleEdgePanning();
    void HandleMouseCameraControl();
    void StartMouseCameraControl();
    void StopMouseCameraControl();
    void RotateCamera(float Value);
    void AngleCamera(float Value);

    // Reset Camera
    void ResetCamera();

    // Simulation speed
    void IncreaseSimulationSpeed();
    void DecreaseSimulationSpeed();
    void ResetSimulationSpeed();
    void UpdateSimulationSpeed();

    // Selection
    void HandleLeftClick();
    AActor* CurrentSelectedActor;

    void UpdateCameraAngle(float CurrentHeight);

    FVector2D CameraMoveInput;
    float CameraZoomInput;
    float CameraRotateInput;
    float CameraAngleInput;
    float CurrentCameraAngle;
    bool bIsMouseCameraControlActive;
    FVector2D LastMousePosition;

    // UI references
    class UTextBlock* SelectionNameText;
    class UScrollBox* SelectionInfoScrollBox;
    class UTextBlock* SimulationSpeedText;
    class UTextBlock* ResourceBarOrganismText;
    class UTextBlock* ResourceBarPlantText;
    class UTextBlock* ResourceBarLifeEssenceText;
    class UProgressBar* ResourceBarEnergyBar;
    class UProgressBar* ResourceBarWaterBar;

    // Helper functions
    void CreateSelectionUI();
    void UpdateSelectionUI(AActor* SelectedActor);
    void HideSelectionUI();
    void CreateSimulationSpeedUI();
    void UpdateSimulationSpeedUI();
    void HideSimulationSpeedUI();
    void CreateResourceBarUI();
    void UpdateResourceBarUI();
    void HideResourceBarUI();
};
