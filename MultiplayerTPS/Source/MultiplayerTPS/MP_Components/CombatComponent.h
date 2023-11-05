// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerTPS/UserWidget/MP_HUD.h"
#include "MultiplayerTPS/Weapon/WeaponTypes.h"
#include "MultiplayerTPS/Types/CombatState.h"
#include "CombatComponent.generated.h"

class AWeapon;
class AMP_PlayerController;
class AMP_HUD;
class AProjectile;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	

public:	
	UCombatComponent();
	friend class AMP_Character;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);
	void SwapWeapons();
	void Reload();

	// AnimNotify
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	// AnimNotify : Called on AniMon_Swap
	UFUNCTION(BlueprintCallable)
	void SwapAttachedWeapon();

	// AnimNotify : Called on AniMon_Swap
	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	// AnimNotify : Shotgun特供装弹函数
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();
	//Shotgun结束装弹
	void JumpToShotgunEnd();

	// AnimNotify : ThrowGrenadeFinished AnimNotify
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	// AnimNotify : GernadeLauncher AnimNotify
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLauncherGrenade(const FVector_NetQuantize& Target);

	// called in AmmoPickup::OnSphereOverlap()
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

	// As label to locally reloading 
	bool bLocallyReloading = false;

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();
public:
	void FireButtonpressed(bool bPressed);

protected:
	// processing fire in server
	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	// Play local fire animation and particles
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(Server, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	// 计算屏幕中心命中结果
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// 设置屏幕准心
	void SetHUDCrosshairs(float DeltaTime);

	// Reload related
	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	// InputAction trigger
	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	// Drop EquippedWeapon
	void DropEquippedWeapon();

	// Attach Actor to Character's RightHand socket
	void AttachActorToRightHand(AActor* ActorToAttach);
	// Attach Actor to Character's RightHand socket
	void AttachActorToLeftHand(AActor* ActorToAttach);
	// Attach Actor to Character's Backpack socket
	void AttachActorToBackpack(AActor* ActorToAttach);

	// Set CarriedAmmo and update HUD
	void UpdateCarriedAmmo();

	// Play EquipWeapon sound
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);

	// Reload Weapon if empty
	void ReloadEmptyWeapon();

	// Alther AttachedGrenade visibility
	void ShowAttachedGrenade(bool bShowGrenade);

	// Equip primary weapon
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	// Equip secondary weapon
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	// Equip specified weapon with key1 and key2
	UFUNCTION(Server, Reliable)
	void ServerEquipSpecifiedWeapon_1();
	UFUNCTION(Server, Reliable)
	void ServerEquipSpecifiedWeapon_2();

private:
	UPROPERTY()
	AMP_Character* Character;
	UPROPERTY()
	AMP_PlayerController* Controller;
	UPROPERTY()
	AMP_HUD* HUD;

	// EquppedWeapon is equals to PrimaryWeapon in design, the SecondaryWeapon is weapon without equipped
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;
	UPROPERTY(Replicated)
	AWeapon* PrimaryWeaponPtr;	// Point to PrimaryWeapon(key Num1)
	UPROPERTY(Replicated)
	AWeapon* SecondaryWeaponPtr;	// Point to SecondaryWeapon(key Num2)

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;
	bool bAimButtonPressed = false;
	UFUNCTION()
	void OnRep_Aiming();

	// initial WalkSpeed
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	// speed of aiming walk
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/**
	* HUD and Crosshairs
	*/
	FCrosshairPackage CrosshairPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	// Calculate every fram
	FVector HitTarget;

	/**
	* Aiming and FOV
	*/
	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/**
	* Automatic fire
	*/
	FTimerHandle FireTimer;

	// Related with Weapon::FireDelay
	bool bCanFire;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// Carried ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	/*	First key is current equipped weapon ammo, 
		Second key is current equipped weapon Max ammo amount
	*/	
	TMap<EWeaponType, TArray<int32>> CarriedAmmoMap;

	/**
	* Numbers of weapon and equipment
	*/
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 StartingARAmmo = 30;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 StartingRocketAmmo = 2;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 StartingPistolAmmo = 24;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 StartingSMGAmmo = 72;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 StartingShotgunAmmo = 18;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 StartingSniperAmmo = 8;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 StartingGrenadeLauncherAmmo = 0;

	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 MaxARAmmo = 180;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 MaxRocketAmmo = 8;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 MaxPistolAmmo = 60;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 MaxSMGAmmo = 288;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 MaxShotgunAmmo = 36;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 MaxSniperAmmo = 8;
	UPROPERTY(EditAnywhere, Category = "Ammunition Amount")
	int32 MaxGrenadeLauncherAmmo = 6;

	UPROPERTY(ReplicatedUsing = OnRep_Grenades,EditAnywhere, Category = "Ammunition Quantity")
	int32 Grenades = 2;
	UPROPERTY(EditAnywhere, Category = "Ammunition Quantity")
	int32 MaxGrenades = 2;

	void InitializeCarriedAmmo();
	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	void UpdateHUDGrenades();

	/**
	* Equipped Weapon state
	*/
	bool bAutomaticFire;



	public:
		FORCEINLINE int32 GetGrenades() const { return Grenades; }	// return number of grenade
		FORCEINLINE TMap<EWeaponType, TArray<int32>> GetCarriedAmmoMap() const { return CarriedAmmoMap; }	// return CarriedAmmoMap
		bool ShouldSwapWeapons();
		FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }	//	return EquippedWeapon
		FORCEINLINE int32 GetCarriedAmmo() const { return CarriedAmmo; }	//	return CarriedAmmo
};
