// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;
class USoundCue;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class MULTIPLAYERTPS_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;	// Display staticmesh

private:
	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;	// Hand pickup range

	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;		// Pickup sound

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;		// 掉落物Niagara效果

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffect;	//	拾取效果

	// Timer to delay collision before Bind delegate
	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();

};
