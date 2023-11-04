// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Hit Projectile"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class ACasing;
class UTexture2D;
class AMP_Character;
class AMP_PlayerController;
class USoundCue;
class UWidgetAnimation;
class UUserWidget;

UCLASS()
class MULTIPLAYERTPS_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;

	void SetHUDAmmo();
	void ShowPickupWidget(bool bShowWidget);
	//Play Fire animation, spend round
	virtual void Fire(const FVector& HitTarget);
	// Drop weapon
	void Dropped();
	// Called when finish relaoded weapon
	virtual void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	USoundCue* EquipSound;

	//	Show Sniper ScopeWidget
	UFUNCTION(BlueprintNativeEvent)
	void ShowScopeWidget(bool bIsAiming);

	/***
	* Enable or disable custom depth
	*/
	void EnableCustomDepth(bool bEnable);

	/**
	* Automatic fire
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.15f;
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	//	true when spawn as default weapon
	bool bDestroyWeapon = false;

	// Weapon  bulllet fire type
	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

	// Calculate HitTarget with random scatter
	FVector TraceEndWithScatter(const FVector& HitTarget);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called in SetWeaponState()
	virtual void OnWeaponStateSet();

	// Called when WeaponState transition to Equipped
	virtual void HandleStateInEquipped();
	// Called when WeaponState transition to EquippedSecondary
	virtual void HandleStateInEquippedSecondary();
	// Called when WeaponState transition to Dropped
	virtual void HandleStateInDropped();
	

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	/**
	* Trace end with scatter
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere)
	float Damage = 40.f;	// Damage per hit

	// whether use Server-Side Rewind
	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	AMP_Character* OwnerCharacter;
	UPROPERTY()
	AMP_PlayerController* OwnerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	// AnimationAsset of weapon skeletal
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;

	// Casing, is also called bullet shell.
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	/**
	* Textures for the weapon crosshairs
	*/
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	/**
	*  Zoomed FOV while aiming
	*/
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Weapon)
	int32 Ammo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	// Ammo amount minuse 1
	// Client-side prediction
	void SpendRound();

	// The number of unprocessed server requests for Ammo
	int32 SequenceAmmo = 0;

	UPROPERTY(EditAnywhere, Category = Weapon)
	int32 MagCapacity;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;


public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UTexture2D* GetCenterCrosshair() const { return CrosshairsCenter; }
	FORCEINLINE UTexture2D* GetLeftCrosshair() const { return CrosshairsLeft; }
	FORCEINLINE UTexture2D* GetRightCrosshair() const { return CrosshairsRight; }
	FORCEINLINE UTexture2D* GetTopCrosshair() const { return CrosshairsTop; }
	FORCEINLINE UTexture2D* GetBottomCrosshair() const { return CrosshairsBottom; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	bool IsMagFull();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
};
