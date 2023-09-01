// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CountdownWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FCountdownOverDelegate)

class UTextBlock;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API UCountdownWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	FCountdownOverDelegate CountdownOverDelegate;

	UFUNCTION(BlueprintCallable)
	void WidgetSetup(int32 CountDownSecond = 10);

	/* call when CountDownSecond become zero*/
	virtual void CountdownOver();

protected:

	/* CountDown Timerhandle callback founction*/
	void UpadteCountdown();

private:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Minute;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Second;

	int32 minute;
	int32 second;

	FTimerHandle CountDownTimer;

};
