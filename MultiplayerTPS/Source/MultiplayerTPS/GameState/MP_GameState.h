// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MP_GameState.generated.h"

class AMP_PlayerState;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AMP_GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const  override;

	/**
	* Handle TopScoringPlayers
	*/
	void UpdateTopScore(AMP_PlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<AMP_PlayerState*> TopScoringPlayers;

	/**
	* Teams
	*/
	TArray<AMP_PlayerState*> RedTeam;
	TArray<AMP_PlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	int RedTeamScore = 0;
	UFUNCTION() 
	void OnRep_RedTeamScore();

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	int BlueTeamScore = 0;
	UFUNCTION()
	void OnRep_BlueTeamScore();

	void RedTeamScores();
	void BlueTeamScores();

	
private:
	float TopScore = 0;
};
