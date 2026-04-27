// Fill out your copyright notice in the Description page of Project Settings.

#include "FMGPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
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

	// Enable replication on this actor
	SetReplicates(true);
	
}

// Called when the game starts or when spawned
void AFMGPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Attach weapon mesh to skeletal mesh's weapon socket
	WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("AutoRifleSocket"));

	// Bind the Fire delegate to the name of from "this" object
	FTimerDelegate FireDelegate;
	FireDelegate.BindUFunction(this, FName("FireWeapon"));
	
	if (userWidget)
	{
		if (UUserWidget* playerHUD = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), userWidget))
		{
			playerHUD->AddToPlayerScreen();
		}
	}

	SetHUDHealth();
	SetHUDCurAmmo();
	SetHUDMaxAmmo();
}

void AFMGPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFMGPlayer, WeaponMesh);
	DOREPLIFETIME(AFMGPlayer, health);
	DOREPLIFETIME(AFMGPlayer, curAmmo);
	DOREPLIFETIME(AFMGPlayer, maxAmmo);
	DOREPLIFETIME(AFMGPlayer, magSize);
}

// Called every frame
void AFMGPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFMGPlayer::SetHUDHealth()
{
	HUDHealth = health / 100.0f;
	//DebugTools::PrintToScreen(4.0f, FColor::Blue, FString::Format(TEXT("Health: {0}"), { HUDHealth }));
}

void AFMGPlayer::SetHUDCurAmmo() 
{
	HUDCurAmmo = curAmmo;
}

void AFMGPlayer::SetHUDMaxAmmo()
{
	HUDMaxAmmo = maxAmmo;
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

void AFMGPlayer::FireWeaponTimer()
{
	if (!openFireGate) return;

	openFireGate = false;
	GetWorldTimerManager().SetTimer(fireTimerHandle, this, &AFMGPlayer::FireWeapon, fireRate, true);
}

void AFMGPlayer::FireWeapon()
{
	if (bIsReloading || GetCharacterMovement()->Velocity.Length() >= 400.0f) return;

	if (!CanFire())
	{
		DebugTools::PrintToScreen(5.0f, FColor::Red, "can't fire");
		ClientPlaySound(magEmptySound);
		return;
	}

	ClientLineTrace();
	ClientSpawnGunshot();
	ClientPlaySound(gunshotSound);
	--curAmmo;
	SetHUDCurAmmo();
}

void AFMGPlayer::StopFiring()
{
	openFireGate = true;
	GetWorldTimerManager().ClearTimer(fireTimerHandle);
}

void AFMGPlayer::ClientLineTrace()
{
	FVector cameraTraceStart = followCamera->GetComponentLocation();
	FVector cameraTraceEnd = followCamera->GetComponentLocation() + followCamera->GetForwardVector() * BULLET_DISTANCE;
	FHitResult hitResult;
	FCollisionQueryParams traceParams;
	traceParams.AddIgnoredActor(this);
	
	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(hitResult, cameraTraceStart, cameraTraceEnd, ECollisionChannel::ECC_Camera, traceParams);

	if (bCameraHit) ROS_LineTrace(cameraTraceStart, cameraTraceEnd);
	//else DrawDebugLine(GetWorld(), bulletTraceStart, bulletTraceEnd, FColor::Red, false, 5.0f, 0, 1.0f);
}

void AFMGPlayer::ROS_LineTrace_Implementation(const FVector& start, const FVector& end)
{
	MC_LineTrace(start, end);
}

void AFMGPlayer::MC_LineTrace_Implementation(const FVector& start, const FVector& end)
{
	FHitResult hitResult;
	FCollisionQueryParams traceParams;
	traceParams.AddIgnoredActor(this);

	// Step 1: Line trace from camera location to where crosshair is aiming
	bool bCameraHit = GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECollisionChannel::ECC_Camera, traceParams);

	// Step 2: Use the weapon muzzle socket location as the start location of bullet trace
	FVector bulletTraceStart = WeaponMesh->GetSocketLocation(TEXT("MuzzleFlash"));
	FVector bulletTraceEnd = bCameraHit ? hitResult.ImpactPoint : hitResult.TraceEnd;

	// Step 3:  If camera's line trace hit something, draw bullet trace to the impact point
	//			Else, draw bullet trace to the end of the line trace
	if (bCameraHit)
	{
		DrawDebugLine(GetWorld(), bulletTraceStart, bulletTraceEnd, FColor::Green, false, 5.0f, 0, 1.0f);
		AActor* hitActor = hitResult.GetActor();
		if (hitActor->ActorHasTag(FName("Player")))
		{
			UGameplayStatics::ApplyPointDamage(
				hitActor,
				20.0f,
				hitResult.ImpactNormal,
				hitResult, 
				GetController(),
				this,
				UDamageType::StaticClass()
			);
		}
		/*
		if (hitActor->GetClass()->ImplementsInterface(UDamageInterface::StaticClass()))
		{
			int32 bulletDamage = 20;
			IDamageInterface::Execute_ApplyDamage(hitActor, bulletDamage);
		}
		*/
	}
	else
	{
		DrawDebugLine(GetWorld(), bulletTraceStart, bulletTraceEnd, FColor::Red, false, 5.0f, 0, 1.0f);
	}
}

void AFMGPlayer::ClientSpawnGunshot()
{
	ROS_SpawnGunshot(WeaponMesh);
}

void AFMGPlayer::ROS_SpawnGunshot_Implementation(USkeletalMeshComponent* attachToComponent)
{
	MC_SpawnGunshot(attachToComponent);
}

void AFMGPlayer::MC_SpawnGunshot_Implementation(USkeletalMeshComponent* attachToComponent)
{
	if (gunshotMuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(gunshotMuzzleEffect, attachToComponent, TEXT("MuzzleFlash"));
	}
}

void AFMGPlayer::ClientPlaySound(USoundBase* sound)
{
	if (sound)
	{
		ROS_PlaySound(sound, GetActorLocation());
	}
}

void AFMGPlayer::ROS_PlaySound_Implementation(USoundBase* sound, const FVector location)
{
	MC_PlaySound(sound, location);
}

void AFMGPlayer::MC_PlaySound_Implementation(USoundBase* sound, const FVector location)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), sound, location, 0.1f);
}

void AFMGPlayer::StartReload()
{
	ClientReload();
}

void AFMGPlayer::ClientReload()
{
	if (!CanReload(curAmmo, magSize, maxAmmo)) 
	{
		DebugTools::PrintToScreen(5.0f, FColor::Green, "client: cant reload");
		return;
	}

	ROS_Reload(GetMesh());
}

void AFMGPlayer::ROS_Reload_Implementation(USkeletalMeshComponent* skm_comp)
{
	if (!CanReload(curAmmo, magSize, maxAmmo))
	{
		DebugTools::PrintToScreen(5.0f, FColor::Green, "server: cant reload");
		return;
	}

	bIsReloading = true;
	MC_Reload(skm_comp);
}

void AFMGPlayer::MC_Reload_Implementation(USkeletalMeshComponent* skm_comp)
{
	DebugTools::PrintToScreen(5.0f, FColor::Orange, "multicast reload called");
	PlayAnimationMontage(reloadAnimMontage);
}

void AFMGPlayer::UpdateAmmo()
{
	// Step 1: Total all ammo in a temp variable
	float totalAmmo = curAmmo + maxAmmo;

	// Step 2: Update curAmmo by loading in what we can
	curAmmo = totalAmmo >= magSize ? magSize : totalAmmo;

	// Step 3: Update maxAmmo
	maxAmmo = totalAmmo - curAmmo;

	//SetHUDCurAmmo();
	//SetHUDMaxAmmo();
}

void AFMGPlayer::StartADS()
{
	if (!CanADS()) return;

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
	return !bIsReloading && !(GetCharacterMovement()->IsFalling()) && (GetCharacterMovement()->Velocity.Length() < 400.0f) && curAmmo > 0;
}

bool AFMGPlayer::CanReload(const int32& curAmmo, const int32& magSize, const int32& maxAmmo)
{
	return !bIsReloading && maxAmmo > 0 && curAmmo != magSize;
}

void AFMGPlayer::OnReloadCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	bIsReloading = false;
	UpdateAmmo();
}

void AFMGPlayer::OnReloadBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	bIsReloading = false;
	UpdateAmmo();
}

void AFMGPlayer::PlayAnimationMontage(UAnimMontage* AnimMontage)
{
	if (GetMesh())
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && AnimMontage)
		{
			AnimInstance->Montage_Play(AnimMontage, 1.0f);

			// Reload BlendOut Delegate - as animation blends out
			FOnMontageEnded BlendOutDelegate;
			BlendOutDelegate.BindUObject(this, &AFMGPlayer::OnReloadBlendOut);
			AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, AnimMontage);

			// Reload Completed Delegate - when animation completely ends
			FOnMontageEnded CompletedDelegate;
			CompletedDelegate.BindUObject(this, &AFMGPlayer::OnReloadCompleted);
			AnimInstance->Montage_SetEndDelegate(CompletedDelegate, AnimMontage);
		}
	}
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

		enhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AFMGPlayer::FireWeaponTimer);
		enhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &AFMGPlayer::StopFiring);
	}
}
