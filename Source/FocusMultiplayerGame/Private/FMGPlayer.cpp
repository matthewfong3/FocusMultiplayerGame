// Fill out your copyright notice in the Description page of Project Settings.


#include "FMGPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AFMGPlayer::AFMGPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set up camera components
	cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	cameraBoom->SetupAttachment(RootComponent);

	followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	followCamera->SetupAttachment(cameraBoom, USpringArmComponent::SocketName);
}

// Called when the game starts or when spawned
void AFMGPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	if (userWidget)
	{
		if (UUserWidget* playerHUD = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), userWidget))
		{
			playerHUD->AddToPlayerScreen();
		}
	}
}

// Called every frame
void AFMGPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFMGPlayer::Move(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Moving"));

	const FVector2D MovementInput = Value.Get<FVector2D>();

	if (GetController() != nullptr) {
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator RotationYaw(0, Rotation.Yaw, 0);
		const FRotator RotationYawRoll(0, Rotation.Yaw, Rotation.Roll);

		const FVector ControlRotationForward = FRotationMatrix(RotationYaw).GetUnitAxis(EAxis::X);
		const FVector ControlRotationRight = FRotationMatrix(RotationYawRoll).GetUnitAxis(EAxis::Y);

		AddMovementInput(ControlRotationForward, MovementInput.Y);
		AddMovementInput(ControlRotationRight, MovementInput.X);
	}
}

void AFMGPlayer::StopMove()
{
	
}

void AFMGPlayer::MouseLook(const FInputActionValue& Value)
{
	const FVector2D MouseInput = Value.Get<FVector2D>();

	if (GetController() != nullptr)
	{
		AddControllerYawInput(MouseInput.X);
		AddControllerPitchInput(MouseInput.Y);
	}
}

void AFMGPlayer::StartJump()
{
	if (GetCharacterMovement()->IsMovingOnGround()) 
	{
		Jump();
	}
}

void AFMGPlayer::StopJump()
{
	StopJumping();
}

// Called to bind functionality to input
void AFMGPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	AddMappingContext();
	BindEnhancedInput(PlayerInputComponent);
}

void AFMGPlayer::AddMappingContext()
{
	UE_LOG(LogTemp, Warning, TEXT("AddMappingContext called"));

	if (APlayerController* playerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		}
	}
}

void AFMGPlayer::BindEnhancedInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFMGPlayer::Move);
		enhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AFMGPlayer::StopMove);

		enhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AFMGPlayer::MouseLook);

		enhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFMGPlayer::StartJump);
		enhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFMGPlayer::StopJump);
	}
}



