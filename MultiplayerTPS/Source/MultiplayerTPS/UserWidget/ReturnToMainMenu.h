 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

class UButton;
class UText;
class UMultiplayerSessionsSubsystem;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/* 显示Widget */
	void MenuSetup();
	/* 关闭Widget */
	void MenuTearDown();

protected:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	
	// Bind delegate with AMP_Character::OnLeftGame
	UFUNCTION()
	void OnPlayerLeftGame();

private:

	UPROPERTY(meta = (BindWidget))
	UButton* ReturnButton;

	// 退出游戏按钮按下触发事件
	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY()
	APlayerController* PlayerController;
};
