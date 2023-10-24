// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/MP_Components/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AMP_Character* MP_Character = Cast<AMP_Character>(OtherActor);
	if (MP_Character)
	{
		UBuffComponent* BuffComponent = MP_Character->GetBuffComponent();
		if (BuffComponent)
		{
			BuffComponent->BuffJump(JumpZVelocityBuff, JumpBuffTime);
		}

	}
	Destroy();
}
