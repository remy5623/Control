// Remy Pijuan 2024.


#include "GravityControlMovementComponent.h"
#include "GameFramework/Character.h"


FVector UGravityControlMovementComponent::NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const
{
	FVector NewGravity = { -980.f, -980.f, -980.f };
	NewGravity *= GravityScaleVector;

	return Super::NewFallVelocity(InitialVelocity, NewGravity, DeltaTime);
}