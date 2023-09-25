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

protected:
	virtual void BeginPlay() override;

private:
	AMP_HUD* MP_HUD;
};
