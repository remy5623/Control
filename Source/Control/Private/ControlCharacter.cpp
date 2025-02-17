// Remy Pijuan 2024.

#include "ControlCharacter.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GravityControlMovementComponent.h"

// Sets default values
AControlCharacter::AControlCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;	// In 3rd person, only camera should get controller rotation
	JumpMaxHoldTime = 0.5f;	// Holding the jump button for up to half a second increases the height of the jump

	GetCharacterMovement()->bAutoActivate = false;
	
	GravityMovement = CreateDefaultSubobject<UGravityControlMovementComponent>(TEXT("Gravity Movement"));
	if (GravityMovement)
	{
		GravityMovement->UpdatedComponent = GetCapsuleComponent();
	}

	// Character movement component settings
	GravityMovement->bUseSeparateBrakingFriction = true;	// Slow the character automatically when not receiving input, simulating friction
	GravityMovement->bOrientRotationToMovement = true;	// Turns the player mesh to face whichever way the character is moving
	GravityMovement->MaxFlySpeed = 2400.f;
	GravityMovement->BrakingDecelerationFlying = 2.f;

	// Setup camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;	// In 3rd person, camera should get control rotation
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;

	// Setup camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Setup the collider for landings
	GroundSurfaceDetector = CreateDefaultSubobject<UBoxComponent>(TEXT("SurfaceDetector"));
	GroundSurfaceDetector->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AControlCharacter::BeginPlay()
{
	Super::BeginPlay();

	RotateToGravityDirection();
}

bool AControlCharacter::CanJumpInternal_Implementation() const
{
	// Ensure that the CharacterMovement state is valid
	bool bJumpIsAllowed = GravityMovement->CanAttemptJump();

	if (bJumpIsAllowed)
	{
		// Ensure JumpHoldTime and JumpCount are valid.
		if (!bWasJumping || GetJumpMaxHoldTime() <= 0.0f)
		{
			if (JumpCurrentCount == 0 && GravityMovement->IsFalling())
			{
				bJumpIsAllowed = JumpCurrentCount + 1 < JumpMaxCount;
			}
			else
			{
				bJumpIsAllowed = JumpCurrentCount < JumpMaxCount;
			}
		}
		else
		{
			// Only consider JumpKeyHoldTime as long as:
			// A) The jump limit hasn't been met OR
			// B) The jump limit has been met AND we were already jumping
			const bool bJumpKeyHeld = (bPressedJump && JumpKeyHoldTime < GetJumpMaxHoldTime());
			bJumpIsAllowed = bJumpKeyHeld &&
				((JumpCurrentCount < JumpMaxCount) || (bWasJumping && JumpCurrentCount == JumpMaxCount));
		}
	}

	return !bIsCrouched && bJumpIsAllowed;
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

	// Set the enhanced input system for the local player from the player controller
	// Add the default mapping context (walking)
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			EnhancedInputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			
			if(EnhancedInputSystem && !WalkingMap.IsNull())
			{
				EnhancedInputSystem->AddMappingContext(WalkingMap.LoadSynchronous(), 0);
			}
		}
	}


	// Bind all actions
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (QuitAction)
		{
			EnhancedInputComponent->BindAction(QuitAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::QuitToDesktop);
		}

		if (LookAction)
			EnhancedInputComponent->BindAction(LookAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::Look);

		if (WalkAction)
			EnhancedInputComponent->BindAction(WalkAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::Walk);

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction.LoadSynchronous(), ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		if (StartFlyingAction)
			EnhancedInputComponent->BindAction(StartFlyingAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::StartFlying);

		if (FlyingMovementAction)
			EnhancedInputComponent->BindAction(FlyingMovementAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::FlyingMovement);

		if (UpwardThrustAction)
			EnhancedInputComponent->BindAction(UpwardThrustAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::AddUpwardThrust);

		if (DownwardThrustAction)
			EnhancedInputComponent->BindAction(DownwardThrustAction.LoadSynchronous(), ETriggerEvent::Triggered, this, &AControlCharacter::AddDownwardThrust);
	}
}

void AControlCharacter::ResetJumpState()
{
	bPressedJump = false;
	bWasJumping = false;
	JumpKeyHoldTime = 0.0f;
	JumpForceTimeRemaining = 0.0f;

	if (GravityMovement && !GravityMovement->IsFalling())
	{
		JumpCurrentCount = 0;
		JumpCurrentCountPreJump = 0;
	}
}

void AControlCharacter::PostInitializeComponents()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_Character_PostInitComponents);

	Super::PostInitializeComponents();

	if (IsValid(this))
	{
		if (GetMesh())
		{
			CacheInitialMeshOffset(GetMesh()->GetRelativeLocation(), GetMesh()->GetRelativeRotation());

			// force animation tick after movement component updates
			if (GetMesh()->PrimaryComponentTick.bCanEverTick&& GravityMovement)
			{
				GetMesh()->PrimaryComponentTick.AddPrerequisite(GravityMovement, GravityMovement->PrimaryComponentTick);
			}
		}

		if (GravityMovement && GetCapsuleComponent())
		{
			GravityMovement->UpdateNavAgent(*GetCapsuleComponent());
		}

		if (Controller == nullptr && GetNetMode() != NM_Client)
		{
			if (GravityMovement && GravityMovement->bRunPhysicsWithNoController)
			{
				GravityMovement->SetDefaultMovementMode();
			}
		}
	}
}

void AControlCharacter::Restart()
{
	Super::Restart();

	JumpCurrentCount = 0;
	JumpCurrentCountPreJump = 0;

	bPressedJump = false;
	ResetJumpState();
	UnCrouch(true);

	if (GravityMovement)
	{
		GravityMovement->SetDefaultMovementMode();
	}
}

void AControlCharacter::QuitToDesktop()
{
	FGenericPlatformMisc::RequestExit(false);
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

void AControlCharacter::StartFlying()
{
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AControlCharacter::ApplyBoost);	// Enable boosting
	GroundSurfaceDetector->OnComponentBeginOverlap.AddDynamic(this, &AControlCharacter::Land);			// Enable landing detection

	StopJumping();
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;

	GetGravityMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	GetGravityMovement()->GravityScale = 0.f;
	GetGravityMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetGravityMovement()->BrakingFriction = 4.f;

	CameraBoom->bEnableCameraLag = false;
	CameraBoom->bEnableCameraRotationLag = false;

	if (EnhancedInputSystem)
		EnhancedInputSystem->AddMappingContext(FlyingMap.LoadSynchronous(), 1);
}

void AControlCharacter::FlyingMovement(const FInputActionValue& FlyValue)
{
	// Convert input to a Vector2D
	FVector2D FlyingVector = FlyValue.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// get forward vector
		const FVector ForwardDirection = Camera->GetForwardVector();

		// get right vector 
		const FVector RightDirection = Camera->GetRightVector();

		// add movement 
		AddMovementInput(ForwardDirection, FlyingVector.Y);
		AddMovementInput(RightDirection, FlyingVector.X);
	}
}

void AControlCharacter::AddUpwardThrust()
{
	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(FVector::UpVector);
	}

}

void AControlCharacter::AddDownwardThrust()
{
	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(FVector::DownVector);
	}
}

void AControlCharacter::Land(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->CanBeBaseForCharacter(this))
	{
		SetActorRotation(FQuat::MakeFromEuler(FVector(0, 0, GetActorRotation().Yaw)));
		StopFlying();
	}
}

void AControlCharacter::StopFlying()
{
	GetCapsuleComponent()->OnComponentBeginOverlap.RemoveDynamic(this, &AControlCharacter::ApplyBoost);	// Disable boosting
	GroundSurfaceDetector->OnComponentBeginOverlap.RemoveDynamic(this, &AControlCharacter::Land);	// Disable landing detection

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;

	GetGravityMovement()->Velocity = FVector::ZeroVector;
	GetGravityMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetGravityMovement()->GravityScale = 1.f;
	GetGravityMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	GetGravityMovement()->BrakingFriction = 0.f;

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;

	if (EnhancedInputSystem)
	{
		EnhancedInputSystem->RemoveMappingContext(FlyingMap.LoadSynchronous());
	}
}

void AControlCharacter::ApplyBoost(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FVector CharLocationRelativeToBoostRing = (OtherActor->GetActorLocation() - GetActorLocation()) - OtherActor->GetActorForwardVector();

	if (CharLocationRelativeToBoostRing.X > 0)
		GetGravityMovement()->Velocity += OtherActor->GetActorForwardVector() * 50000.f;
	else
		GetGravityMovement()->Velocity -= OtherActor->GetActorForwardVector() * 50000.f;
}

void AControlCharacter::RotateToGravityDirection()
{
	FVector DownVector = (GetActorUpVector() * -1).GetSafeNormal();
	FVector GravityVector = GetGravityMovement()->GravityScaleVector.GetSafeNormal();

	if (DownVector != GravityVector)
	{
		GetGravityMovement()->Velocity = FVector::ZeroVector;
		SetActorRotation(FQuat::FindBetween(DownVector, GravityVector * -1).Rotator());
	}
}

void AControlCharacter::AlterGravity(FVector NewGravityDirection)
{
	GetGravityMovement()->GravityScaleVector = NewGravityDirection;
	RotateToGravityDirection();
}

void AControlCharacter::CheckJumpInput(float DeltaTime)
{
	JumpCurrentCountPreJump = JumpCurrentCount;

	if (GravityMovement)
	{
		if (bPressedJump)
		{
			// If this is the first jump and we're already falling,
			// then increment the JumpCount to compensate.
			const bool bFirstJump = JumpCurrentCount == 0;
			if (bFirstJump && GravityMovement->IsFalling())
			{
				JumpCurrentCount++;
			}

			const bool bDidJump = CanJump() && GravityMovement->DoJump(bClientUpdating);
			if (bDidJump)
			{
				// Transition from not (actively) jumping to jumping.
				if (!bWasJumping)
				{
					JumpCurrentCount++;
					JumpForceTimeRemaining = GetJumpMaxHoldTime();
					OnJumped();
				}
			}

			bWasJumping = bDidJump;
		}
	}
}

UGravityControlMovementComponent* AControlCharacter::GetGravityMovement() const
{
	return 	GravityMovement;
;
}