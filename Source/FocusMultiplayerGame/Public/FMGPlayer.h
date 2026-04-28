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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
public:
	// Weapon-related fields
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = true))
	int32 curAmmo = 30;
	UPROPERTY(Replicated)
	int32 magSize = 30;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = true))
	int32 maxAmmo = 90;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = true))
	float health = 100.0f;

private:
	// CONSTANTS
	const float BULLET_DISTANCE = 10000.0f;
	const float DEFAULT_WALK_SPEED = 500.0f;
	const float DEFAULT_ADS_SPEED = 200.0f;
	const float fireRate = 0.12f;

	bool openFireGate = true;
	FTimerHandle fireTimerHandle;

	// Conditional Bools
	UPROPERTY(Replicated)
	bool bIsReloading;
	UPROPERTY(Replicated)
	bool bIsADS;

	// Pure Methods
	bool CanADS();
	bool CanFire();
	bool CanReload(int32 curA, int32 maxA, int32 magS);
private:
	// Camera Fields
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> cameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> followCamera;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
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

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Particle System", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> gunshotMuzzleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> gunshotSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = true))
	TObjectPtr<USoundBase> magEmptySound;
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
	UFUNCTION()
	void StartReload();
	// ADS
	void StartADS();
	void CancelADS();

private:
	void SetupCameraSettings();

	UFUNCTION()
	void OnReloadCompleted(UAnimMontage* Montage, bool bInterrupted, int32 curA, int32 maxA, int32 magS);
	UFUNCTION()
	void OnReloadBlendOut(UAnimMontage* Montage, bool bInterrupted, int32 curA, int32 maxA, int32 magS);

	// Fire weapon line trace
	UFUNCTION()
	void FireWeapon();
	void ClientLineTrace();
	UFUNCTION(Server, Reliable)
	void ROS_LineTrace(const FVector& start, const FVector& end);
	UFUNCTION(NetMulticast, Reliable)
	void MC_LineTrace(const FVector& start, const FVector& end);

	// Reload weapon
	UFUNCTION()
	void ClientReload();
	UFUNCTION(Server, Reliable)
	void ROS_Reload(USkeletalMeshComponent* skm_comp, int32 curA, int32 maxA, int32 magS);
	UFUNCTION(NetMulticast, Reliable)
	void MC_Reload(USkeletalMeshComponent* skm_comp, int32 curA, int32 maxA, int32 magS);

	UFUNCTION()
	void OnRep_curAmmo();
	void UpdateAmmo(int32 curA, int32 maxA, int32 magS);

	// Play Gunshot Emitter
	void ClientSpawnGunshot();
	UFUNCTION(Server, Reliable)
	void ROS_SpawnGunshot(USkeletalMeshComponent* attachToComponent);
	UFUNCTION(NetMulticast, Reliable)
	void MC_SpawnGunshot(USkeletalMeshComponent* attachToComponent);

	// Play SFX
	void ClientPlaySound(USoundBase* sound);
	UFUNCTION(Server, Reliable)
	void ROS_PlaySound(USoundBase* sound, const FVector location);
	UFUNCTION(NetMulticast, Reliable)
	void MC_PlaySound(USoundBase* sound, const FVector location);

	// Play Animation Montage
	UFUNCTION()
	void PlayReloadAnimMontage(UAnimMontage* AnimMontage, int32 curA, int32 maxA, int32 magS);

};
