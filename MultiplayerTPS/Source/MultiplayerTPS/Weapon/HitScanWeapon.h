// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	//	Line trace scan , apply damage if successed
	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere)
	float Damage = 40.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticle;
};
