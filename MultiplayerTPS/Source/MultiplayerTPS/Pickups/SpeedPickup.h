// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
	
protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;	// Buff speed of stand

	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;	//Buff speed of crouch

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 30.f;		//Buff duration time
};
