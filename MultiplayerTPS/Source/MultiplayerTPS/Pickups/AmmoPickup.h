// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "MultiplayerTPS/Weapon/WeaponTypes.h"

#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
public:
	AAmmoPickup();

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = AmmoProperties)
	int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere, Category = AmmoProperties)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, Category = AmmoProperties)
	bool bAmmoGroup = 0;

	UPROPERTY(EditAnywhere, Category = AmmoProperties)
	float GroupAmmoPick = 0.5f;	//	Percentage  of Recovered Ammo Group
};

