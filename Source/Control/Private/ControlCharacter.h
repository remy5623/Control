// Remy Pijuan 2024.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
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

private:
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSystem;

protected:
	/** Camera Components */

	// A spring arm acts as a virtual camera boom, smoothing 3rd person camera movement
	UPROPERTY(VisibleAnywhere, Category=Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category=Camera)
	UCameraComponent* Camera;

	
	/** Flight Components */
	// A volume to detect when there is a surface below to land on.
	UPROPERTY(VisibleAnywhere, Category=Flight)
	UBoxComponent* GroundSurfaceDetector;


	/** Input Variables */

	// The mapping context for walking movement
	UPROPERTY(EditDefaultsOnly, Category="Input|Maps")
	TSoftObjectPtr<UInputMappingContext> WalkingMap;

	// The mapping context for flying movement
	UPROPERTY(EditDefaultsOnly, Category="Input|Maps")
	UInputMappingContext* FlyingMap;

	// The input action for quitting the game
	UPROPERTY(EditDefaultsOnly, Category="Input|Settings")
	TSoftObjectPtr<UInputAction> QuitAction;

	// The input action for camera movement
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	TSoftObjectPtr<UInputAction> LookAction;

	// The input action for walking movement
	UPROPERTY(EditDefaultsOnly, Category="Input|Walk")
	TSoftObjectPtr<UInputAction> WalkAction;

	// The input action for jumping
	UPROPERTY(EditDefaultsOnly, Category="Input|Walk")
	TSoftObjectPtr<UInputAction> JumpAction;
	
	// The input action to start flying (from walking)
	UPROPERTY(EditDefaultsOnly, Category="Input|Flight")
	TSoftObjectPtr<UInputAction> StartFlyingAction;

	// The input action for flying movement
	UPROPERTY(EditDefaultsOnly, Category="Input|Flight")
	TSoftObjectPtr<UInputAction> FlyingMovementAction;

	UPROPERTY(EditDefaultsonly, Category="Input|Flight")
	TSoftObjectPtr<UInputAction> UpwardThrustAction;

	UPROPERTY(EditDefaultsOnly, Category="Input|Flight")
	TSoftObjectPtr<UInputAction> DownwardThrustAction;

private:
	/** Settings Functions */

	// Quit the game
	void QuitToDesktop();


	/** Movement Functions */

	// Controls camera movement
	UFUNCTION()
	void Look(const FInputActionValue& LookValue);

	// Controls player movement when in walking mode
	UFUNCTION()
	void Walk(const FInputActionValue& WalkValue);


	/** Flying Functions */
	UFUNCTION()
	void StartFlying();
	
	UFUNCTION()
	void FlyingMovement(const FInputActionValue& FlyValue);

	UFUNCTION()
	void AddUpwardThrust();

	UFUNCTION()
	void AddDownwardThrust();

	UFUNCTION()
	void Land(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void StopFlying();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
