// Remy Pijuan 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GravityControlMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class UGravityControlMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector GravityScaleVector = { 0, 0, 1 };

private:
	/**
	 * Compute new falling velocity from given velocity and gravity. Applies the limits of the current Physics Volume's TerminalVelocity.
	 */
	virtual FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const override;
};
