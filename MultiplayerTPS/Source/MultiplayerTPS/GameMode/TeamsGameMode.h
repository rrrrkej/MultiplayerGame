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
	ATeamsGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float  CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	void PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController) override;
protected:
	virtual void HandleMatchHasStarted() override;

	UPROPERTY(EditAnywhere)
	int32 MaxScore = 6;

public:
	FORCEINLINE int32 GetMaxScore() const { return MaxScore; }

};
