// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class MULTIPLAYERTPS_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	// Handle Trailsystem disappear
	void StartDestroyTimer();
	void DestroyTimerFinished();

	// Spawn TrailSystemComponent
	void SpawnTrailSystem();

	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	// Components
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh; // 飞行物网格体
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent; // 飞行物运动部件

	/**
	* Apply Damage
	*/
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles; // 命中粒子

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound; // 命中音效

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox; // 飞行物碰撞

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem; // 弹道粒子

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;
	 
	UPROPERTY(EditAnywhere)
	float DamageInnerRaius; //范围武器上海内径

	UPROPERTY(EditAnywhere)
	float DamageOuterRaius; //范围武器伤害外径

private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer; //	无实体的子弹效果

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
	
};
