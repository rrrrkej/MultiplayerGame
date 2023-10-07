// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MP_PlayerController.generated.h"

class AMP_HUD;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AMP_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
private:
	UPROPERTY()
	AMP_HUD* MP_HUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};
