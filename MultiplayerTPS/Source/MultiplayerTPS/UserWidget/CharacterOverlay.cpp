// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"
#include "Components/VerticalBox.h"
#include "Engine.h"

void UCharacterOverlay::NativeConstruct()
{
	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("NativeConstruct()")));
	}
	Super::NativeConstruct();
	//if (TeamScoreBox)
	//{
	//	TeamScoreBox->SetVisibility(ESlateVisibility::Hidden);
	//}
}
