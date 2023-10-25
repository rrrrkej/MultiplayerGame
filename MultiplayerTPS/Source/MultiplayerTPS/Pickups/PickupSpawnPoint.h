// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

class APickup;

UCLASS()
class MULTIPLAYERTPS_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;

	// Spawn Pickup Actor
	void SpawnPickup();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

	// Call SpawnPickup() when timer elapsed
	void SpawnPickupTimerFinished();

	// Classes to spawn
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<APickup>> PickupClasses;

	// store the SpawnedPickup
	UPROPERTY()
	APickup* SpawnedPickup;

private:
	FTimerHandle SpawnPickupTimer;
	
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin = 3.f;
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax = 5.f;

};
