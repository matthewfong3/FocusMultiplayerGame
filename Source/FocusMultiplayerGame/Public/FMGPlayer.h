// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FMGPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;

UCLASS()
class FOCUSMULTIPLAYERGAME_API AFMGPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFMGPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// CONSTANTS
	const float BULLET_DISTANCE = 10000.0f;
	const float DEFAULT_WALK_SPEED = 500.0f;
	const float DEFAULT_ADS_SPEED = 200.0f;
	const float fireRate = 0.12f;

	int32 curAmmo = 30;
	bool openFireGate = true;
	FTimerHandle fireTimerHandle;

	// Conditional Bools
	bool bIsReloading;
	bool bIsADS;

	// Pure Methods
	bool CanADS();
	bool CanFire();
private:
	// Camera Fields
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> cameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> followCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	// Input Mapping Contexts
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Mapping Context", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Mapping Context", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> MouseLookInputMappingContext;

	// Enhanced Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ADSAction;

	UPROPERTY(EditAnywhere, Category = "Input", meta = (AllowPrivateAcccess = "true"))
	TObjectPtr<UInputAction> ReloadAction;

	// Reload Animation Montage
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> reloadAnimMontage;

	// HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> userWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particle System", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> gunshotMuzzleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> gunshotSound;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Input Mapping Context
	UFUNCTION(BlueprintCallable, Category = "Input Mapping Context")
	void AddMappingContext();
	void BindEnhancedInput(UInputComponent* PlayerInputComponent);

	// Enhanced Input Action Functions
	void Move(const FInputActionValue& Value);
	void MouseLook(const FInputActionValue& Value);
	// Jump
	void StartJump();
	void StopJump();
	// Shoot
	void FireWeaponTimer();
	void StopFiring();
	// Reload
	void StartReload();
	// ADS
	void StartADS();
	void CancelADS();

private:
	void SetupCameraSettings();
	UFUNCTION()
	void OnReloadCompleted(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void OnReloadBlendOut(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void FireWeapon();
	void FireLineTrace();
	void SpawnGunshotMuzzleEffect();
	void PlayGunshotSound();
	void PlayAnimationMontage(UAnimMontage* AnimMontage);
};
