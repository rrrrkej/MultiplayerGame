// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

class UCountdownWidget;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	// 人数到达要求后开始倒计时
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	// 倒计时结束后调用
	UFUNCTION()
	void GameStart(FString GameModeName);

	//UCountDownWidget* BP_CountDownWidget;
	
	UPROPERTY(EditAnywhere, Category = Widget, meta = (AllPrivateAccess = "true"))
	TSubclassOf<UCountdownWidget> BP_UCountdownWidgetClass;

	UCountdownWidget* CountdownWidget;

	// Countdown time start once players enough
	UPROPERTY(EditAnywhere)
	float TimeToStart = 3;

};
