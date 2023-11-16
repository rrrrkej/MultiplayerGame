// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GamePlayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

#include "WeaponTypes.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/MP_Components/LagCompensationComponent.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "MultiplayerTPS/DebugHeader.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	//DrawDebugSphere(GetWorld(), HitTarget, 16.f, 12.f, FColor::Red, true);
	//	Get InstigatorController used in ApplyDamage
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
	
		//	Apply Damage
		AMP_Character* HitCharacter = Cast<AMP_Character>(FireHit.GetActor());
		if (HitCharacter &&  InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

					UGameplayStatics::ApplyDamage(
						HitCharacter,
						DamageToCause,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					); 
				}
				if(!HasAuthority() && bUseServerSideRewind)
				{
					OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMP_Character>(OwnerPawn) : OwnerCharacter;
					OwnerController = OwnerController == nullptr ? Cast<AMP_PlayerController>(InstigatorController) : OwnerController;
					if (OwnerCharacter && OwnerController && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
					{
						OwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
							HitCharacter,
							Start,
							FireHit.ImpactPoint,
							OwnerController->GetServerTime() - OwnerController->SingleTripTime
						);
						AMP_Character* Character = Cast<AMP_Character>(InstigatorController->GetPawn());
					}
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

		//Spawn HitTarget soundcue
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
	
		// Spawn MuzzleFlash
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		// Play Fire soundcue
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		//	Line trace scan
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}
		//DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12.f, FColor::Red, true);
		// Spawn bullet trail
		if (BeamParticle)
		{
			UParticleSystemComponent* Beam =
				UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticle,
					TraceStart,
					FRotator::ZeroRotator,
					true
				);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}



