// Fill out your copyright notice in the Description page of Project Settings.

#include "FMGPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DamageInterface.h"
#include "PrintStrings.h"

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

	SetupCameraSettings();

	// Set up weapon mesh component
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	WeaponMesh->SetupAttachment(GetMesh());
}

// Called when the game starts or when spawned
void AFMGPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Attach weapon mesh to skeletal mesh's weapon socket
	WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("AutoRifleSocket"));
	
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

void AFMGPlayer::MouseLook(const FInputActionValue& Value)
{
	const FVector2D MouseInput = Value.Get<FVector2D>();

	if (GetController() != nullptr)
	{
		AddControllerYawInput(MouseInput.X);
		AddControllerPitchInput(MouseInput.Y);
	}
}

void AFMGPlayer::SetupCameraSettings()
{
	// Tell pawn character to use the controller's yaw rotation
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;
	// Tell the camera boom to use pawn's control rotation
	cameraBoom->bUsePawnControlRotation = true;
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

void AFMGPlayer::FireWeapon()
{
	if (!CanFire())
	{
		DebugTools::PrintToScreen(5.0f, FColor::Red, "can't fire");
		return;
	}

	// Add a fire rate delay
	FireLineTrace();
	SpawnGunshotMuzzleEffect();
	PlayGunshotSound();
}

void AFMGPlayer::FireLineTrace()
{
	FVector cameraTraceStart = followCamera->GetComponentLocation();
	FVector cameraTraceEnd = followCamera->GetComponentLocation() + followCamera->GetForwardVector() * BULLET_DISTANCE;
	FHitResult hitResult;
	FCollisionQueryParams traceParams;
	traceParams.AddIgnoredActor(this);

	// Step 1: Line trace from camera location to where crosshair is aiming
	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(hitResult, cameraTraceStart, cameraTraceEnd, ECollisionChannel::ECC_Camera, traceParams);

	// Step 2: Use the weapon muzzle socket location as the start location of bullet trace
	FVector bulletTraceStart = WeaponMesh->GetSocketLocation(TEXT("MuzzleSocket"));
	FVector bulletTraceEnd = bCameraHit ? hitResult.ImpactPoint : hitResult.TraceEnd;

	// Step 3:  If camera's line trace hit something, draw bullet trace to the impact point
	//			Else, draw bullet trace to the end of the line trace
	if (bCameraHit)
	{
		DrawDebugLine(GetWorld(), bulletTraceStart, bulletTraceEnd, FColor::Green, false, 5.0f, 0, 1.0f);
		AActor* hitActor = hitResult.GetActor();
		if (hitActor->GetClass()->ImplementsInterface(UDamageInterface::StaticClass()))
		{
			int32 bulletDamage = 20;
			IDamageInterface::Execute_ApplyDamage(hitActor, bulletDamage);
		}

	}
	else
	{
		DrawDebugLine(GetWorld(), bulletTraceStart, bulletTraceEnd, FColor::Red, false, 5.0f, 0, 1.0f);
	}
}

void AFMGPlayer::SpawnGunshotMuzzleEffect()
{
	if (gunshotMuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(gunshotMuzzleEffect, WeaponMesh, TEXT("MuzzleFlash"));
	}
}

void AFMGPlayer::PlayGunshotSound()
{
	if (gunshotSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), gunshotSound, GetActorLocation());
	}
}

void AFMGPlayer::StartReload()
{
	
}

void AFMGPlayer::StartADS()
{
	if (!CanADS())
	{
		return;
	}

	bIsADS = true;

	followCamera->SetFieldOfView(FMath::FInterpTo(followCamera->FieldOfView, 60.f, GetWorld()->GetDeltaSeconds(), 120.0f));
	GetCharacterMovement()->MaxWalkSpeed = DEFAULT_ADS_SPEED;
}

void AFMGPlayer::CancelADS()
{
	bIsADS = false;

	followCamera->SetFieldOfView(FMath::FInterpTo(followCamera->FieldOfView, 90.f, GetWorld()->GetDeltaSeconds(), 120.0f));
	GetCharacterMovement()->MaxWalkSpeed = DEFAULT_WALK_SPEED;
}

bool AFMGPlayer::CanADS() {
	return !bIsReloading && !(GetCharacterMovement()->IsFalling());
}

bool AFMGPlayer::CanFire()
{	
	// REVISIT THIS: COMPILE ERROR
	//const float vL = GetCharacterMovement()->Velocity.Length;
	return !bIsReloading && !(GetCharacterMovement()->IsFalling()) && curAmmo > 0; // (vL < 400.0f)
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
	if (APlayerController* playerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			subsystem->AddMappingContext(DefaultInputMappingContext, 0);
			subsystem->AddMappingContext(MouseLookInputMappingContext, 0);
		}
	}
}

void AFMGPlayer::BindEnhancedInput(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		enhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFMGPlayer::Move);

		enhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AFMGPlayer::MouseLook);

		enhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AFMGPlayer::StartJump);
		enhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AFMGPlayer::StopJump);

		enhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AFMGPlayer::StartReload);

		enhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Triggered, this, &AFMGPlayer::StartADS);
		enhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Completed, this, &AFMGPlayer::CancelADS);
		enhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Canceled, this, &AFMGPlayer::CancelADS);

		enhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AFMGPlayer::FireWeapon);
	}
}
