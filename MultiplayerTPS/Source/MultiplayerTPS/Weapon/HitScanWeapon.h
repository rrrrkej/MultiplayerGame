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


protected:
	

	// Spawn BeamParticle and calculate HitResult
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles; // ����Ч��

	UPROPERTY(EditAnywhere)
	USoundCue* HitSound; // ������Ч

	UPROPERTY(EditAnywhere)
	float Damage = 40.f; // ���������˺�

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticle; // �����켣

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash; // ǹ�ڻ���

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound; // ������Ч


};
