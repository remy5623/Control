// Rémy Pijuan 2024.


#include "ControlCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
AControlCharacter::AControlCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Allow up/down camera movement - yaw (left/right) is enabled by default
	bUseControllerRotationPitch = true;

	// Setup camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AControlCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AControlCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/** Called to bind functionality to input
*   Adds the default input mapping context (walking), and its input actions
*/
void AControlCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Get the enhanced input system for the local player from the player controller
	// Add the default mapping context (walking)
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				EnhancedInputSystem->AddMappingContext(WalkingMap.LoadSynchronous(), 0);
			}
		}
	}

	// Bind actions for the walking mapping context
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (LookAction)
			EnhancedInputComponent->BindAction(LookAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::Look);

		if (WalkAction)
			EnhancedInputComponent->BindAction(WalkAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::Walk);
	}
}

/** Apply input from the x-axis to the camera's yaw
*   Apply input from the y-axis to the camera's pitch
*/
void AControlCharacter::Look(const FInputActionValue& LookValue)
{
	// Convert input to a Vector2D
	FVector2D LookAxisVector = LookValue.Get<FVector2D>();

	if (Controller)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

/** Apply input from the x-axis to lateral movement
*   Apply input from the y-axis to forward movement
*/
void AControlCharacter::Walk(const FInputActionValue& WalkValue)
{
	// Convert input to a Vector2D
	FVector2D WalkingVector = WalkValue.Get<FVector2D>();

	if (Controller)
	{
		// add movement
		AddMovementInput(GetActorRightVector(), WalkingVector.X);
		AddMovementInput(GetActorForwardVector(), WalkingVector.Y);
	}
}