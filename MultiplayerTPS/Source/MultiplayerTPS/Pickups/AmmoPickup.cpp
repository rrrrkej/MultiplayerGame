// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/MP_Components/CombatComponent.h"
#include "MultiplayerTPS/Weapon/WeaponTypes.h"
#include "MultiplayerTPS/DebugHeader.h"

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
			// Add  all kinds of ammo
			if (bAmmoGroup)
			{
				for (int32 i = 0; i < (int32)EWeaponType::EWT_MAX; ++i)
				{
					WeaponType = static_cast<EWeaponType>(i);
					AmmoAmount = (int32)round(CombatComponent->GetCarriedAmmoMap()[WeaponType][1] * GroupAmmoPick);
					CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
				}
			}
			else
			{
				CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
			}
		}
		
	}
	Destroy();
}
