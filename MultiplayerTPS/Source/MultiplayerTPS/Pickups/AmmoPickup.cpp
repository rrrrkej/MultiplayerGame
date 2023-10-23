// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/MP_Components/CombatComponent.h"

AAmmoPickup::AAmmoPickup()
{
	PickupMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	PickupMesh->MarkRenderStateDirty();
	PickupMesh->SetRenderCustomDepth(true);
}

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AMP_Character* MP_Character = Cast<AMP_Character>(OtherActor);
	if (MP_Character)
	{
		UCombatComponent* CombatComponent = MP_Character->GetCombatComponent();
		if (CombatComponent)
		{
			CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	Destroy();
}
