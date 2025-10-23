#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Selectable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class USelectable : public UInterface
{
	GENERATED_BODY()
};

class THEMEANINGOFLIFE_API ISelectable
{
	GENERATED_BODY()

public:
    // Called when this object is selected
    virtual void OnSelected() = 0;

    // Called when this object is deselected
    virtual void OnDeselected() = 0;

    // Get display name
    virtual FString GetDisplayName() = 0;

    // Get info to display (returns array of property names and values)
    virtual TArray<TPair<FString, FString>> GetDisplayInfo() = 0;
};
