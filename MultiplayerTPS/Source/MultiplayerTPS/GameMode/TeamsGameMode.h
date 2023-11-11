// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MP_GameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API ATeamsGameMode : public AMP_GameMode
{
	GENERATED_BODY()
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float  CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
protected:
	virtual void HandleMatchHasStarted() override;
};
