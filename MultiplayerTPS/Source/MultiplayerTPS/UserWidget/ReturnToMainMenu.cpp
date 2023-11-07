// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"

#include "MultiplayerSessionsSubsystem.h"
#include "MultiplayerTPS/Character/MP_Character.h"

bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	
	return true;
}

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	// 绑定按钮功能
	if (ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}

	// 绑定会话广播
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	// 解绑按钮功能
	if (ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}

	// 解绑会话广播
	if (MultiplayerSessionsSubsystem && !MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);	// 禁用按钮，防止重复触发（spam）

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			AMP_Character*  MP_Character = Cast<AMP_Character>(FirstPlayerController->GetPawn());
			if (MP_Character)
			{
				MP_Character->ServerLeaveGame();	// 播放elim动画，处理GameState数据， 最后触发this->OnPlayerLeftGame()
				MP_Character->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
			}
			else // 如果角色处于重生状态则重置按钮等待重生
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

void UReturnToMainMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

