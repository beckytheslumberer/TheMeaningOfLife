#include "LifeSimGameMode.h"
#include "LifeSimPlayerController.h"
#include "LifeSimCameraPawn.h"

ALifeSimGameMode::ALifeSimGameMode()
{
    // Set default pawn and controller classes
    DefaultPawnClass = ALifeSimCameraPawn::StaticClass();
    PlayerControllerClass = ALifeSimPlayerController::StaticClass();

    UE_LOG(LogTemp, Warning, TEXT("LifeSimGameMode initialized"));
}