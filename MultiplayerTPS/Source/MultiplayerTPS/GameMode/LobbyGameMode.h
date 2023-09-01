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
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	void GameStart();

	//UCountDownWidget* BP_CountDownWidget;
	
	UPROPERTY(EditAnywhere, Category = Widget, meta = (AllPrivateAccess = "true"))
	TSubclassOf<UCountdownWidget> BP_UCountdownWidgetClass;

	UCountdownWidget* CountdownWidget;
};
