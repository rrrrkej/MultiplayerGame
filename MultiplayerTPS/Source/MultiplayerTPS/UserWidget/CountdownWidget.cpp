// Fill out your copyright notice in the Description page of Project Settings.


#include "CountdownWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UCountdownWidget::WidgetSetup(int32 CountDownSecond)
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);

	// caculate remain time
	minute = CountDownSecond / 60;
	second = CountDownSecond % 60;

	Minute->SetText(FText::AsNumber(minute));
	Second->SetText(FText::AsNumber(second));

	GetWorld()->GetTimerManager().SetTimer(CountDownTimer, this, &UCountdownWidget::UpadteCountdown, 1.f, true);
}

void UCountdownWidget::CountdownOver()
{
	RemoveFromParent();
	CountdownOverDelegate.Broadcast(); // 没绑定也是安全的
}

void UCountdownWidget::UpadteCountdown()
{
	if (second < 0)
	{
		second = 0;
	}

	second--;
	if (minute > 0 && second < 0)
	{
		second = 59;
		minute--;
	}

	if (minute < 0 || second < 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountDownTimer);
		CountdownOver();
	}

	Minute->SetText(FText::AsNumber(minute));
	Second->SetText(FText::AsNumber(second));
}
