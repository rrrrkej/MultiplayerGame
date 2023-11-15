// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "../UserWidget/CountdownWidget.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerSessionsSubSystem.h"
#include "TimerManager.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);
										  
		if (NumberOfPlayers >= Subsystem->DesiredNumPublicConnections)
		{
			if (BP_UCountdownWidgetClass)
				CountdownWidget = CreateWidget<UCountdownWidget>(GetWorld(), BP_UCountdownWidgetClass);
			CountdownWidget->WidgetSetup(3);
			//CountdownWidget->CountdownOverDelegate.AddUObject(this, &ALobbyGameMode::GameStart);

			// Countdown timer to ServerTravel
			FTimerHandle GameStartTimer;
			FTimerDelegate GameStartDelegate;
			GameStartDelegate.BindUFunction(this, FName("GameStart"), Subsystem->DesiredMatchType);
			GetWorldTimerManager().SetTimer(
				GameStartTimer,
				GameStartDelegate,
				TimeToStart,
				false
			);
		}
	}
}

void ALobbyGameMode::GameStart(FString GameModeName)
{
	CountdownWidget->RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{	
		//UE_LOG(LogTemp, Warning, TEXT("GameStart()"));
		bUseSeamlessTravel = true;
		if(GameModeName == "FreeForAll")
		{
			World->ServerTravel(FString("/Game/Maps/MP_Map?listen"));
		}
		else if (GameModeName == "Teams")
		{
			World->ServerTravel(FString("/Game/Maps//MP_Map?listen"));
		}
		else if (GameModeName == "CaptureTheFlag")
		{
			World->ServerTravel(FString("/Game/Maps//MP_Map?listen"));
		}
	}
}
