// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	friend class AMP_Character;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	//Set the text of DisplayText
	void SetDisplayText(FString TextToDisplay);
	//Outer call, Set the role
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);
protected:
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;
};
