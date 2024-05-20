// Rémy Pijuan 2024.


#include "ControlCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AControlCharacter::AControlCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;	// In 3rd person, only camera should get controller rotation
	JumpMaxHoldTime = 0.5f;	// Holding the jump button for up to half a second increases the height of the jump
	
	// Character movement component settings
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;	// Slow the character automatically when not receiving input, simulating friction
	GetCharacterMovement()->bOrientRotationToMovement = true;	// Turns the player mesh to face whichever way the character is moving

	// Setup camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;	// In 3rd person, camera should get control rotation
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;

	// Setup camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
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

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		if (FlyAction)
			EnhancedInputComponent->BindAction(FlyAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::Fly);
	}
}

/** Apply input from the x-axis to the camera's yaw
*   Apply input from the y-axis to the camera's pitch
*	From Epic's Third Person Template
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

/** Move the player relative to the camera
*	From Epic's Third Person Template
*/
void AControlCharacter::Walk(const FInputActionValue& WalkValue)
{
	// Convert input to a Vector2D
	FVector2D WalkingVector = WalkValue.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, WalkingVector.Y);
		AddMovementInput(RightDirection, WalkingVector.X);
	}
}

void AControlCharacter::Fly()
{
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	GetCharacterMovement()->GravityScale = 0.f;
}