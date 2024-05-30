// Remy Pijuan 2024.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "GravityControlMovementComponent.h"
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
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> EnhancedInputSystem;

protected:
	/** Camera Components */

	// A spring arm acts as a virtual camera boom, smoothing 3rd person camera movement
	UPROPERTY(VisibleAnywhere, Category=Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category=Camera)
	TObjectPtr<UCameraComponent> Camera;

	
	/** Flight Components */
	// A volume to detect when there is a surface below to land on.
	UPROPERTY(VisibleAnywhere, Category=Flight)
	TObjectPtr<UBoxComponent> GroundSurfaceDetector;

	
	/** Gravity Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Movement, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGravityControlMovementComponent> GravityMovement;


	/** Input Variables */

	// The mapping context for walking movement
	UPROPERTY(EditDefaultsOnly, Category="Input|Maps")
	TSoftObjectPtr<UInputMappingContext> WalkingMap;

	// The mapping context for flying movement
	UPROPERTY(EditDefaultsOnly, Category="Input|Maps")
	TSoftObjectPtr<UInputMappingContext> FlyingMap;

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

	UFUNCTION()
	void ApplyBoost(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * Rewritten to use GravityControlMovementComponent
	 */
	virtual bool CanJumpInternal_Implementation() const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Marks character as not trying to jump
	*   Overridden to replace refs to CharacterMovement with GravityMovement
	*/
	virtual void ResetJumpState() override;

	// Overridden to replace refs to CharacterMovement with GravityMovement
	virtual void PostInitializeComponents() override;
	virtual void Restart() override;

	/** Trigger jump if jump button has been pressed.
	*   Overridden to use GravityControlMovementComponent instead of CharacterMovementComponent
	*/
	virtual void CheckJumpInput(float DeltaTime) override;

	UGravityControlMovementComponent* GetGravityMovement() const;
};
