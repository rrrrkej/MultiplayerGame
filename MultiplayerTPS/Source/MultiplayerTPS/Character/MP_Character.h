// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MultiplayerTPS/Types/TurningInPlace.h"
#include "MultiplayerTPS/Interfaces/InteractWithCrosshairsInterface.h"
#include "MP_Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;

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
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void AimOffset(float DeltaTime);
private:

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); 
	
	UPROPERTY(VisibleAnywhere)
	UCombatComponent* CombatComponent;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	/* aimoffset animation*/
	float AO_Yaw;
	float InterpAO_Yaw; //used in turning in place
	float AO_Pitch;
	FRotator StartingAimRotation;

	//Turning in place
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	// Hide Camera if character get too close to camera
	void HideCamera();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

#pragma region AnimMontage
private:
	UPROPERTY(EditAnywhere, Category = AnimMontage)
	UAnimMontage* FireWeaponMontage;

public:
	void PlayFireMontage(bool bAiming);
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
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for jump */
	void Jump(const FInputActionValue& Value);

	/** Called for Interaction */
	void Interaction(const FInputActionValue& Value);

	/** Called for Crouch */
	void Crouch(const FInputActionValue& Value);

	/**Called for Aim */
	void Aim(const FInputActionValue& Value);
	void EndAim(const FInputActionValue& Value);

	/** Called for Fire*/
	void Fire(const FInputActionValue& Value);
	void EndFire(const FInputActionValue& Value);

#pragma endregion

public:	
	//be accessed by Weapon class
	void SetOverlappingWeapon(AWeapon* Weapon);

	//be accessed by CombatComponent class
	bool IsWeaponEquipped();
	bool IsAiming();
	FVector GetHitTarget() const;

	// be accessed by AnimInstance class
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
