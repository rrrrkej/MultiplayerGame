// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MultiplayerTPS/Types/TurningInPlace.h"
#include "MultiplayerTPS/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "MultiplayerTPS/Types/CombatState.h"
#include "MP_Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class AMP_PlayerController;
class AController;
class USoundCue;
class AMP_PlayerState;
class UBuffComponent;
UCLASS()
class MULTIPLAYERTPS_API AMP_Character : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMP_Character();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	// Handle what happens when the player gets eliminated
	void Elim();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();
	virtual void Destroyed() override;

	//	true when character state be uncontrollable
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	//	Update  HUD
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	//	Spawn default weapon at beginning
	void SpawnDefaultWeapon();

	// Handle weapon state when character is elimmed
	void HandleWeaponWhenElimed(AWeapon* Weapon);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	// fix TurnInPlace anim in Simulated Proxies
	void SimuProxiesTurn();
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);
	
	// Poll for any relevant classes and initizlize HUDoverlay class
	void PollInit();
	// Founction for Rotate In Place feature
	void RotateInPlace(float DeltaTime);

	// Set health progress bar in OverheadWidget
	void SetOverheadHealth();
private:

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidgetComponent;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UBuffComponent* BuffComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	/* aimoffset animation*/
	float AO_Yaw;
	float InterpAO_Yaw; //used in turning in place
	float AO_Pitch;
	FRotator StartingAimRotation;

	//Turning in place, Used as a condition for changing animation states in AnimBlueprint
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	// Hide Camera if character get too close to camera
	void HideCamera();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 2.f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/**
	* Character health
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Health,  Category = "Player Stats")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Shield,  Category = "Player Stats")
	float Shield = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);
	UFUNCTION()
	void OnRep_Shield(float LastShield);

	AMP_PlayerController* MP_PlayerController;

	bool bElimmed = false; // true when Elimmed

	/**
	* Timerhandle of RespawnCharacter
	*/
	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3;

	void ElimTimerFinished();

	/**
	* Dissolve effect
	*/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/**
	* Elim bot effect
	*/
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ElimBotSound;

	UPROPERTY()
	AMP_PlayerState* MP_PlayerState;

	/**
	* Grenade
	*/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/**
	* Default weaopn
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
#pragma region AnimMontage
private:
	UPROPERTY(EditAnywhere, Category = AnimMontage)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = AnimMontage)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = AnimMontage)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = AnimMontage)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = AnimMontage)
	UAnimMontage* ThrowGrenadeMontage;


public:
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
#pragma endregion

#pragma region InputBinding

private:
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ThrowGrenadeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SecondaryWeapon;
protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for jump */
	void JumpPressed(const FInputActionValue& Value);

	/** Called for Interaction */
	void Interaction(const FInputActionValue& Value);

	/** Called for Crouch */
	void CrouchPressed(const FInputActionValue& Value);

	/**Called for Aim */
	void Aim(const FInputActionValue& Value);
	void EndAim(const FInputActionValue& Value);

	/** Called for Fire*/
	void Fire(const FInputActionValue& Value);
	void EndFire(const FInputActionValue& Value);

	/* Called for Reload*/
	void Reload(const FInputActionValue& Value);

	/* Called for throw grenade*/
	void ThrowGrenade(const FInputActionValue& Value);

	/* swap to specified weapon*/
	void EquipPrimaryWeapon(const FInputActionValue& Value);
	void EquipSecondaryWeapon(const FInputActionValue& Value);

#pragma endregion

public:	
	//be accessed by Weapon class
	void SetOverlappingWeapon(AWeapon* Weapon);

	//be accessed by CombatComponent class
	bool IsWeaponEquipped();
	bool IsAiming();
	FVector GetHitTarget() const;

	// be accessed by AnimInstance class
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; } // return AO_Yaw
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; } // return AO_Pitch
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; } // return TurningInPlace
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; } // return FollowCamera
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; } // return bRotateRootBone
	FORCEINLINE bool IsElimmed() const { return bElimmed; } // return bElimmed
	FORCEINLINE float GetHealth() const{ return Health; } // return Health
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }	// Set Health
	FORCEINLINE float GetMaxHealth() const{ return MaxHealth; } // return MaxHealth
	FORCEINLINE float GetShield() const { return Shield; }	//return Shield
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }	// Set Shield
	FORCEINLINE float GetMaxShield() { return MaxShield; }	// return MaxShield
	ECombatState GetCombatState() const;
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; } // return CombatComponent
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; } // return bDisableGameplay
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; } // return ReloadMontage
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; } // return AttachedGrenade
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; } // return buff component
};
