#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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
    class UUserWidget* SelectionInfoWidget;

    UPROPERTY()
    class UUserWidget* SimulationSpeedWidget;

private:
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

    // Helper functions
    void CreateSelectionUI();
    void UpdateSelectionUI(AActor* SelectedActor);
    void HideSelectionUI();
    void CreateSimulationSpeedUI();
    void UpdateSimulationSpeedUI();
    void HideSimulationSpeedUI();
};
