#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageInterface.generated.h"

// This class does not need to be modified
UINTERFACE(MinimalAPI, Blueprintable)
class UDamageInterface : public UInterface
{
	GENERATED_BODY()
};

class IDamageInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This si the class that will be inherited to implement this interface
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Interface")
	void ApplyDamage(const int dmg);
};