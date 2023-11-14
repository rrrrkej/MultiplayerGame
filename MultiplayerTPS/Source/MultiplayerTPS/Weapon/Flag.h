// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();

	virtual void Dropped() override;

	// reset transform while reach FlagZone
	void ResetFlag();

protected:
	virtual void BeginPlay() override;

	// Called when WeaponState transition to Equipped
	virtual void HandleStateInEquipped() override;

	// Called when WeaponState transition to Dropped
	virtual void HandleStateInDropped() override;


private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	// save transform at beginning
	FTransform InitialTransform;

public:
	FORCEINLINE FTransform GetInitialLocation() const { return InitialTransform; }
};
