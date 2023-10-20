// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "Kismet/GamePlayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	//	Play FireAnim and update ammo
	AWeapon::Fire(HitTarget);

	//	Get InstigatorController used in ApplyDamage
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<AMP_Character*, uint32> HitMap;	//	命中对象
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;	//	命中结果
			WeaponTraceHit(Start, HitTarget, FireHit);		// 计算命中结果

			AMP_Character* MP_Character = Cast<AMP_Character>(FireHit.GetActor());
			if (MP_Character && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(MP_Character))
				{
					HitMap[MP_Character]++;
				}
				else
				{
					HitMap.Emplace(MP_Character, 1);
				}

				//Spawn HitTarget soundcue if hit character
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-.5f, .5f)
					);
				}
			}

			//	Spawn impact ParticleSystem
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}

			
		}
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}
