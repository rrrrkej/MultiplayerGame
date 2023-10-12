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
	// Called when Character's HP down to zero
	virtual void PlayerEliminated(AMP_Character* ElimmedCharacter, AMP_PlayerController* VictimController, AMP_PlayerController* AttackerController);
	// Called when ElimTimer finished
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
