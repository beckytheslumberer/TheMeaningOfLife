#include "LifeSimPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/ProgressBar.h"
#include "Selectable.h"
#include "ResourceComponent.h"

ALifeSimPlayerController::ALifeSimPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create resource component
    MyResourceComponent = CreateDefaultSubobject<UResourceComponent>(TEXT("MyResourceComponent"));

    // Load widget classes
    static ConstructorHelpers::FClassFinder<UUserWidget> SelectionWidget(TEXT("/Game/UI/WBP_SelectionInfo"));
    if (SelectionWidget.Succeeded())
    {
        SelectionInfoWidgetClass = SelectionWidget.Class;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WBP_SelectionInfo not found"));
    }

    static ConstructorHelpers::FClassFinder<UUserWidget> InfoRow(TEXT("/Game/UI/WBP_InfoRow"));
    if (InfoRow.Succeeded())
    {
        InfoRowWidgetClass = InfoRow.Class;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WBP_InfoRow not found"));
    }

    static ConstructorHelpers::FClassFinder<UUserWidget> SimulationSpeed(TEXT("/Game/UI/WBP_SimulationSpeed"));
    if (SimulationSpeed.Succeeded())
    {
        SimulationSpeedWidgetClass = SimulationSpeed.Class;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WBP_SimulationSpeed not found"));
    }

    static ConstructorHelpers::FClassFinder<UUserWidget> ResourceBar(TEXT("/Game/UI/WBP_ResourceBar"));
    if (ResourceBar.Succeeded())
    {
        ResourceBarWidgetClass = ResourceBar.Class;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("WBP_ResourceBar not found"));
    }

    // Selection
    CurrentSelectedActor = nullptr;
    SelectionInfoWidget = nullptr;
    SelectionNameText = nullptr;
    SelectionInfoScrollBox = nullptr;
    SimulationSpeedWidget = nullptr;
    SimulationSpeedText = nullptr;
    ResourceBarWidget = nullptr;
    ResourceBarOrganismText = nullptr;
    ResourceBarPlantText = nullptr;
    ResourceBarLifeEssenceText = nullptr;
    ResourceBarEnergyBar = nullptr;
    ResourceBarWaterBar = nullptr;

    // Camera defaults
    CameraMoveSpeed = 2000.0f;
    CameraZoomSpeed = 20000.0f;
    CameraRotateSpeed = 90.0f; 
    CameraAngleSpeed = 45.0f;
    MouseRotateSensitivity = 0.3f;
    MouseAngleSensitivity = 0.3f;
    MinZoomHeight = 500.0f;
    MaxZoomHeight = 5000.0f;
    MinCameraAngle = -15.0f; // At min height (500): -15 degrees (more horizontal/landscape)
    MaxCameraAngle = -75.0f; // At max height (5000): -75 degrees (more top-down)
    DefaultCameraHeight = 2000.0f;
    DefaultCameraAngle = -60.0f;

    // Edge panning (Off by default)
    EdgePanBorderSize = 20.0f;
    bUseEdgePanning = false;

    // Simulation speed defaults
    CurrentSimulationSpeed = 1.0f;
    MinSimulationSpeed = 0.0625f;
    MaxSimulationSpeed = 16.0f;

    // Initialize input
    CameraMoveInput = FVector2D::ZeroVector;
    CameraZoomInput = 0.0f;
    CameraRotateInput = 0.0f;
    CameraAngleInput = 0.0f;
    CurrentCameraAngle = DefaultCameraAngle;
    bIsMouseCameraControlActive = false;
    LastMousePosition = FVector2D::ZeroVector;
}

void ALifeSimPlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    CreateSimulationSpeedUI();
    CreateSelectionUI();
    CreateResourceBarUI();

    UpdateSimulationSpeed();

    UE_LOG(LogTemp, Warning, TEXT("Player Controller initialized"));
}

void ALifeSimPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Camera movement (WASD)
    InputComponent->BindAxis("MoveForward", this, &ALifeSimPlayerController::MoveCameraForward);
    InputComponent->BindAxis("MoveRight", this, &ALifeSimPlayerController::MoveCameraRight);

    // Camera zoom (Mouse Wheel)
    InputComponent->BindAxis("ZoomCamera", this, &ALifeSimPlayerController::ZoomCamera);

    // Mouse camera control (Middle Mouse Button)
    InputComponent->BindAction("MouseCameraControl", IE_Pressed, this, &ALifeSimPlayerController::StartMouseCameraControl);
    InputComponent->BindAction("MouseCameraControl", IE_Released, this, &ALifeSimPlayerController::StopMouseCameraControl);

    // Camera rotation (Left Right Arrow Keys)    
    // Camera angle (Up Down Arrow Keys)
    InputComponent->BindAxis("RotateCamera", this, &ALifeSimPlayerController::RotateCamera);
    InputComponent->BindAxis("AngleCamera", this, &ALifeSimPlayerController::AngleCamera);

    // Camera reset (CTRL + Space)
    InputComponent->BindAction("ResetCamera", IE_Pressed, this, &ALifeSimPlayerController::ResetCamera);

    // Simulation speed controls
    InputComponent->BindAction("IncreaseSpeed", IE_Pressed, this, &ALifeSimPlayerController::IncreaseSimulationSpeed);
    InputComponent->BindAction("DecreaseSpeed", IE_Pressed, this, &ALifeSimPlayerController::DecreaseSimulationSpeed);
    InputComponent->BindAction("ResetSpeed", IE_Pressed, this, &ALifeSimPlayerController::ResetSimulationSpeed);

    // Selection
    InputComponent->BindAction("LeftClick", IE_Pressed, this, &ALifeSimPlayerController::HandleLeftClick);
}

void ALifeSimPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update Resource UI
    UpdateResourceBarUI();

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    //Get real delta time (unaffected by time dilation)
    float RealDeltaTime = DeltaTime / CurrentSimulationSpeed;

    // Handle mouse camera control
    if (bIsMouseCameraControlActive)
    {
        HandleMouseCameraControl();
    }

    // Handle edge panning
    if (bUseEdgePanning)
    {
        HandleEdgePanning();
    }

    // Apply camera movement (use RealDeltaTime)
    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector ForwardMovement = ControlledPawn->GetActorForwardVector() * CameraMoveInput.X * CameraMoveSpeed * RealDeltaTime;
    FVector RightMovement = ControlledPawn->GetActorRightVector() * CameraMoveInput.Y * CameraMoveSpeed * RealDeltaTime;

    // Keep movement on XY plane
    ForwardMovement.Z = 0;
    RightMovement.Z = 0;

    FVector NewLocation = CurrentLocation + ForwardMovement + RightMovement;
    
    // Apply zoom
    float NewHeight = FMath::Clamp(CurrentLocation.Z + (CameraZoomInput * CameraZoomSpeed * RealDeltaTime),
        MinZoomHeight, MaxZoomHeight);
    NewLocation.Z = NewHeight;

    ControlledPawn->SetActorLocation(NewLocation);

    // Apply camera rotation (yaw) from keyboard
    if (FMath::Abs(CameraRotateInput) > 0.01f)
    {
        FRotator CurrentRotation = ControlledPawn->GetActorRotation();
        CurrentRotation.Yaw += CameraRotateInput * CameraRotateSpeed * RealDeltaTime;
        ControlledPawn->SetActorRotation(CurrentRotation);
    }

    // Apply camera angle adjustment (pitch) from keyboard
    if (FMath::Abs(CameraAngleInput) > 0.01f)
    {
        CurrentCameraAngle += CameraAngleInput * CameraAngleSpeed * RealDeltaTime;
        CurrentCameraAngle = FMath::Clamp(CurrentCameraAngle, MaxCameraAngle, MinCameraAngle);
    }

    // Adjust camera angle based on zoom level
    UpdateCameraAngle(NewHeight);

    // Reset inputs for next frame
    CameraMoveInput = FVector2D::ZeroVector;
    CameraZoomInput = 0.0f;
    CameraRotateInput = 0.0f;
    CameraAngleInput = 0.0f;
}

void ALifeSimPlayerController::MoveCameraForward(float Value)
{
    CameraMoveInput.X = Value;
}

void ALifeSimPlayerController::MoveCameraRight(float Value)
{
    CameraMoveInput.Y = Value;
}

void ALifeSimPlayerController::ZoomCamera(float Value)
{
    CameraZoomInput = -Value; // Invert so scroll up = zoom in
}

void ALifeSimPlayerController::RotateCamera(float Value) {
    CameraRotateInput = Value;
}

void ALifeSimPlayerController::AngleCamera(float Value) {
    CameraAngleInput = Value;
}

void ALifeSimPlayerController::StartMouseCameraControl()
{
    bIsMouseCameraControlActive = true;

    // Get initial mouse position
    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    LastMousePosition = FVector2D(MouseX, MouseY);

    // Hide curser while dragging
    bShowMouseCursor = false;
}

void ALifeSimPlayerController::StopMouseCameraControl()
{
    bIsMouseCameraControlActive = false;

    // Show cursor again
    bShowMouseCursor = true;
}

void ALifeSimPlayerController::HandleMouseCameraControl()
{
    float MouseX, MouseY;
    if (!GetMousePosition(MouseX, MouseY))
        return;

    FVector2D CurrentMousePosition(MouseX, MouseY);
    FVector2D MouseDelta = CurrentMousePosition - LastMousePosition;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    // Rotate camera (yaw) based on horizontal mouse movement
    FRotator CurrentRotation = ControlledPawn->GetActorRotation();
    CurrentRotation.Yaw += MouseDelta.X * MouseRotateSensitivity;
    ControlledPawn->SetActorRotation(CurrentRotation);

    // Adjust camera angle (pitch) based on vertical mouse movement
    CurrentCameraAngle -= MouseDelta.Y * MouseAngleSensitivity;
    CurrentCameraAngle = FMath::Clamp(CurrentCameraAngle, MaxCameraAngle, MinCameraAngle);

    LastMousePosition = CurrentMousePosition;
}

void ALifeSimPlayerController::HandleEdgePanning()
{
    float MouseX, MouseY;
    if (!GetMousePosition(MouseX, MouseY))
        return;

    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);

    // Check edges and add movement
    if (MouseX < EdgePanBorderSize)
    {
        CameraMoveInput.Y -= 1.0f; // Pan left
    }
    else if (MouseX > ViewportSizeX - EdgePanBorderSize)
    {
        CameraMoveInput.Y += 1.0f; // Pan right
    }

    if (MouseY < EdgePanBorderSize)
    {
        CameraMoveInput.X += 1.0f; // Pan forward
    }
    else if (MouseY > ViewportSizeY - EdgePanBorderSize)
    {
        CameraMoveInput.X -= 1.0f; // Pan backward
    }
}

void ALifeSimPlayerController::IncreaseSimulationSpeed()
{
    CurrentSimulationSpeed = FMath::Clamp(CurrentSimulationSpeed * 2.0f, MinSimulationSpeed, MaxSimulationSpeed);
    UpdateSimulationSpeed();
    UE_LOG(LogTemp, Warning, TEXT("Simulation speed increased to: %fx"), CurrentSimulationSpeed);
}

void ALifeSimPlayerController::DecreaseSimulationSpeed()
{
    CurrentSimulationSpeed = FMath::Clamp(CurrentSimulationSpeed * 0.5f, MinSimulationSpeed, MaxSimulationSpeed);
    UpdateSimulationSpeed();
    UE_LOG(LogTemp, Warning, TEXT("Simulation speed decreased to: %fx"), CurrentSimulationSpeed);
}

void ALifeSimPlayerController::ResetSimulationSpeed()
{
    CurrentSimulationSpeed = 1.0f;
    UpdateSimulationSpeed();
    UE_LOG(LogTemp, Warning, TEXT("Simulation speed reset to: 1x"));
}

void ALifeSimPlayerController::UpdateSimulationSpeed()
{
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::SetGlobalTimeDilation(World, CurrentSimulationSpeed);
    }

    UpdateSimulationSpeedUI();
}

void ALifeSimPlayerController::HandleLeftClick()
{
    if (bIsMouseCameraControlActive)
        return;

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    bool bHit = GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult);

    AActor* HitActor = bHit ? HitResult.GetActor() : nullptr;
    ISelectable* SelectableActor = nullptr;

    if (HitActor)
    {
        SelectableActor = Cast<ISelectable>(HitActor);
    }

    if (SelectableActor)
    {
        // Deselect previous if different
        if (CurrentSelectedActor && CurrentSelectedActor != HitActor)
        {
            ISelectable* PreviousSelectable = Cast<ISelectable>(CurrentSelectedActor);
            if (PreviousSelectable)
            {
                PreviousSelectable->OnDeselected();
            }
        }

        // Select new
        CurrentSelectedActor = HitActor;
        SelectableActor->OnSelected();

        // Update UI
        UpdateSelectionUI(HitActor);
    }
    else
    {
        // Deselect
        if (CurrentSelectedActor)
        {
            ISelectable* PreviousSelectable = Cast<ISelectable>(CurrentSelectedActor);
            if (PreviousSelectable)
            {
                PreviousSelectable->OnDeselected();
            }
            CurrentSelectedActor = nullptr;
        }

        // Hide UI
        HideSelectionUI();
    }
}

void ALifeSimPlayerController::UpdateCameraAngle(float CurrentHeight)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    // Find the camera component
    UCameraComponent* Camera = ControlledPawn->FindComponentByClass<UCameraComponent>();
    if (!Camera)
        return;

    // Use the manual angle if it's been adjusted, otherwise auto-adjust based on zoom
    if (CurrentCameraAngle != DefaultCameraAngle)
    {
        FRotator NewRotation = Camera->GetRelativeRotation();
        NewRotation.Pitch = CurrentCameraAngle;
        Camera->SetRelativeRotation(NewRotation);
        return;
    }

    // Calculate angle based on height
    float HeightRatio = (CurrentHeight - MinZoomHeight) / (MaxZoomHeight - MinZoomHeight);
    float TargetPitch = FMath::Lerp(MinCameraAngle, MaxCameraAngle, HeightRatio);

    FRotator NewRotation = Camera->GetRelativeRotation();
    NewRotation.Pitch = TargetPitch;
    Camera->SetRelativeRotation(NewRotation);
}

void ALifeSimPlayerController::ResetCamera()
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    // Reset height
    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    CurrentLocation.Z = DefaultCameraHeight;
    ControlledPawn->SetActorLocation(CurrentLocation);

    // Reset rotation (yaw)
    FRotator CurrentRotation = ControlledPawn->GetActorRotation();
    CurrentRotation.Yaw = 0.0f; // Face north
    ControlledPawn->SetActorRotation(CurrentRotation);

    // Reset angle
    CurrentCameraAngle = DefaultCameraAngle;

    UE_LOG(LogTemp, Warning, TEXT("Camera reset to default position"));
}

void ALifeSimPlayerController::CreateSelectionUI()
{
    if (SelectionInfoWidgetClass)
    {
        SelectionInfoWidget = CreateWidget<UUserWidget>(this, SelectionInfoWidgetClass);

        if (SelectionInfoWidget)
        {
            SelectionInfoWidget->AddToViewport();

            // Get references to the UI elements
            SelectionNameText = Cast<UTextBlock>(SelectionInfoWidget->GetWidgetFromName(TEXT("NameText")));
            SelectionInfoScrollBox = Cast<UScrollBox>(SelectionInfoWidget->GetWidgetFromName(TEXT("InfoScrollBox")));

            // Start hidden
            SelectionInfoWidget->SetVisibility(ESlateVisibility::Hidden);

            UE_LOG(LogTemp, Warning, TEXT("Selection UI created successfully"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SelectionInfoWidgetClass not loaded!"));
    }
}

void ALifeSimPlayerController::UpdateSelectionUI(AActor* SelectedActor)
{
    if (!SelectionInfoWidget || !SelectedActor)
        return;

    ISelectable* Selectable = Cast<ISelectable>(SelectedActor);
    if (!Selectable)
        return;

    // Show the widget
    SelectionInfoWidget->SetVisibility(ESlateVisibility::Visible);

    // Update name
    if (SelectionNameText)
    {
        SelectionNameText->SetText(FText::FromString(Selectable->GetDisplayName()));
    }

    // Clear and rebuild info list
    if (SelectionInfoScrollBox)
    {
        SelectionInfoScrollBox->ClearChildren();

        // Get info from the selected actor
        TArray<TPair<FString, FString>> Info = Selectable->GetDisplayInfo();

        // Use the InfoRowWidgetClass we loaded in constructor
        if (InfoRowWidgetClass)
        {
            for (const TPair<FString, FString>& InfoPair : Info)
            {
                UUserWidget* RowWidget = CreateWidget<UUserWidget>(this, InfoRowWidgetClass);

                if (RowWidget)
                {
                    // Set the text values
                    UTextBlock* PropNameText = Cast<UTextBlock>(RowWidget->GetWidgetFromName(TEXT("PropertyNameText")));
                    UTextBlock* PropValueText = Cast<UTextBlock>(RowWidget->GetWidgetFromName(TEXT("PropertyValueText")));

                    if (PropNameText)
                    {
                        PropNameText->SetText(FText::FromString(InfoPair.Key + TEXT(":")));
                    }

                    if (PropValueText)
                    {
                        PropValueText->SetText(FText::FromString(InfoPair.Value));
                    }

                    SelectionInfoScrollBox->AddChild(RowWidget);
                }
            }
        }
    }
}

void ALifeSimPlayerController::HideSelectionUI()
{
    if (SelectionInfoWidget)
    {
        SelectionInfoWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void ALifeSimPlayerController::CreateSimulationSpeedUI()
{
    if (SimulationSpeedWidgetClass)
    {
        SimulationSpeedWidget = CreateWidget<UUserWidget>(this, SimulationSpeedWidgetClass);

        if (SimulationSpeedWidget)
        {
            SimulationSpeedWidget->AddToViewport();

            // Get references to the UI elements
            SimulationSpeedText = Cast<UTextBlock>(SimulationSpeedWidget->GetWidgetFromName(TEXT("SimulationSpeedText")));

            // Start hidden
            SimulationSpeedWidget->SetVisibility(ESlateVisibility::Hidden);

            UE_LOG(LogTemp, Warning, TEXT("Simulation Speed UI created successfully"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SimulationSpeedWidgetClass not loaded!"));
    }
}

void ALifeSimPlayerController::UpdateSimulationSpeedUI()
{
    if (!SimulationSpeedWidget) 
        return;

    // Show the widget
    SimulationSpeedWidget->SetVisibility(ESlateVisibility::Visible);

    // Update text
    if (SimulationSpeedText)
    {
        SimulationSpeedText->SetText(FText::FromString("Simulation Speed: " + FString::SanitizeFloat(CurrentSimulationSpeed) + "x"));
    }
}

void ALifeSimPlayerController::HideSimulationSpeedUI()
{
    if (SelectionInfoWidget)
    {
        SelectionInfoWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void ALifeSimPlayerController::CreateResourceBarUI()
{
    if (ResourceBarWidgetClass)
    {
        ResourceBarWidget = CreateWidget<UUserWidget>(this, ResourceBarWidgetClass);

        if (ResourceBarWidget)
        {
            ResourceBarWidget->AddToViewport();

            // Get references to the UI elements
            ResourceBarOrganismText = Cast<UTextBlock>(ResourceBarWidget->GetWidgetFromName(TEXT("OrganismTextBlock")));
            ResourceBarPlantText = Cast<UTextBlock>(ResourceBarWidget->GetWidgetFromName(TEXT("PlantTextBlock")));
            ResourceBarLifeEssenceText = Cast<UTextBlock>(ResourceBarWidget->GetWidgetFromName(TEXT("LifeEssenceTextBlock")));
            ResourceBarEnergyBar = Cast<UProgressBar>(ResourceBarWidget->GetWidgetFromName(TEXT("ResourceEnergyBar")));
            ResourceBarWaterBar = Cast<UProgressBar>(ResourceBarWidget->GetWidgetFromName(TEXT("ResourceWaterBar")));

            // Start hidden
            ResourceBarWidget->SetVisibility(ESlateVisibility::Hidden);

            UE_LOG(LogTemp, Warning, TEXT("Resource Bar UI created successfully"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ResourceBarWidgetClass not loaded!"));
    }
}

void ALifeSimPlayerController::UpdateResourceBarUI()
{
    if (!ResourceBarWidget)
        return;

    // Show the widget
    ResourceBarWidget->SetVisibility(ESlateVisibility::Visible);

    // Update text
    if (ResourceBarOrganismText)
    {
        ResourceBarOrganismText->SetText(FText::FromString(MyResourceComponent->OrganismInfoToString()));
    }
    if (ResourceBarPlantText)
    {
        ResourceBarPlantText->SetText(FText::FromString(MyResourceComponent->PlantInfoToString()));
    }
    if (ResourceBarLifeEssenceText)
    {
        ResourceBarLifeEssenceText->SetText(FText::FromString(MyResourceComponent->LifeEssenceInfoToString()));
    }

    // Update progress bars
    if (ResourceBarEnergyBar)
    {
        ResourceBarEnergyBar->SetPercent(MyResourceComponent->GetEnergyPercent());
    }
    if (ResourceBarWaterBar)
    {
        ResourceBarWaterBar->SetPercent(MyResourceComponent->GetWaterPercent());
    }
}

void ALifeSimPlayerController::HideResourceBarUI()
{
    if (ResourceBarWidget)
    {
        ResourceBarWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}