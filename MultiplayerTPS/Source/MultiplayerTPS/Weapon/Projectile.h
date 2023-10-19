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
	UStaticMeshComponent* ProjectileMesh; // ������������
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent; // �������˶�����

	/**
	* Apply Damage
	*/
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles; // ��������

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound; // ������Ч

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox; // ��������ײ

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem; // ��������

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;
	 
	UPROPERTY(EditAnywhere)
	float DamageInnerRaius; //��Χ�����Ϻ��ھ�

	UPROPERTY(EditAnywhere)
	float DamageOuterRaius; //��Χ�����˺��⾶

private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer; //	��ʵ����ӵ�Ч��

	UPROPERTY()
	UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
	
};
