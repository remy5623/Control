// Rémy Pijuan 2024.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "ControlCharacter.generated.h"

UCLASS()
class AControlCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AControlCharacter();

protected:
	UPROPERTY(VisibleAnywhere, Category=Camera)
	UCameraComponent* Camera;

	// The mapping context for walking movement
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputMappingContext> WalkingMap;

	// The input action for camera movement
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputAction> LookAction;

	// The input action for walking movement
	UPROPERTY(EditDefaultsOnly, Category=Input)
	TSoftObjectPtr<UInputAction> WalkAction;

private:
	// Controls camera movement
	UFUNCTION()
	void Look(const FInputActionValue& LookValue);

	// Controls player movement when in walking mode
	UFUNCTION()
	void Walk(const FInputActionValue& WalkValue);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
