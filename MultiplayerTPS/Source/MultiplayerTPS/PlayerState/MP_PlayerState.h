// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MP_PlayerState.generated.h"

class AMP_Character;
class AMP_PlayerController;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AMP_PlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);

private:
	AMP_Character* Character;
	AMP_PlayerController* Controller;
};
