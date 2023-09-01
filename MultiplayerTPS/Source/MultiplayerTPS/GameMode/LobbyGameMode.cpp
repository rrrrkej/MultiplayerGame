// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "../UserWidget/CountdownWidget.h"
#include "Kismet/GameplayStatics.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (NumberOfPlayers == 2)
	{
		if(BP_UCountdownWidgetClass)
		CountdownWidget = CreateWidget<UCountdownWidget>(GetWorld(), BP_UCountdownWidgetClass);
		CountdownWidget->WidgetSetup(5);
		CountdownWidget->CountdownOverDelegate.AddUObject(this, &ALobbyGameMode::GameStart);
	}
}

void ALobbyGameMode::GameStart()
{
	UWorld* World = GetWorld();
	if (World)
	{	
		UE_LOG(LogTemp, Warning, TEXT("GameStart()"));
		bUseSeamlessTravel = true;
		World->ServerTravel(FString("/Game/Maps/MP_Map?listen"));
	}
}
