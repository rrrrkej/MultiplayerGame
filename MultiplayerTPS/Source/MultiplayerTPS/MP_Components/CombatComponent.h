// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class AMP_Character;
class AMP_PlayerController;
class AMP_HUD;

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
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	// 计算屏幕中心命中结果
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	// 设置屏幕准心
	void SetHUDCrosshairs(float DeltaTime);
private:
	AMP_Character* Character;
	AMP_PlayerController* Controller;
	AMP_HUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bAiming;

	// initial WalkSpeed
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	// speed of aiming walk
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;
};
