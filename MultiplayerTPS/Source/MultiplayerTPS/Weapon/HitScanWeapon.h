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
	UParticleSystem* ImpactParticles; // 命中效果

	UPROPERTY(EditAnywhere)
	USoundCue* HitSound; // 命中音效

	UPROPERTY(EditAnywhere)
	float Damage = 40.f; // 单次命中伤害

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticle; // 弹道轨迹

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash; // 枪口火焰

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound; // 开火音效


};
