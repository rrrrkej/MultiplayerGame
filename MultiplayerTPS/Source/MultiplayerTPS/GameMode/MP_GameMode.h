// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MP_GameMode.generated.h"

class AMP_Character;
class AMP_PlayerController;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AMP_GameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	// Called when Character's HP down to zero
	virtual void PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController);
	// Called when ElimTimer finished
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
};
