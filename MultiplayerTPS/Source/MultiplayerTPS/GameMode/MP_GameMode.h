// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MP_GameMode.generated.h"

class AMP_Character;
class AMP_PlayerController;

namespace MatchState
{
	extern MULTIPLAYERTPS_API const FName Cooldown; //	Match duration has been reached. Display winner and begin cooldown Timer.
}

/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AMP_GameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AMP_GameMode();

	virtual void Tick(float DeltaTime) override;
	// Called at MP_Character::ReceiveDamage()
	virtual void PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController);
	// Called when ElimTimer finished
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	// Called by AMP_Character::ServerLeaveGame, Quit game
	void PlayerLeftGame(class AMP_PlayerState* PlayerLeaving);

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;	// 热身模式时间

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;	// 比赛时间

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;	// 比赛结束后，自动开始下一场比赛前的冷却时间

	float LevelStartingTime = 0; // 关卡（map）开始时间

	bool bTeamsMode = false; // 是否是TeamsGameMode
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	FORCEINLINE void FinishTeamsGame() { SetMatchState(MatchState::Cooldown); }
};
