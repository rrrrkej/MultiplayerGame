// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureFlagGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API ACaptureFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(
		AMP_Character* ElimmedCharacter,
		AMP_PlayerController* VictimController,
		AMP_PlayerController* AttackerController) override;

	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};
