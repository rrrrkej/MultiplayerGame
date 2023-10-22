// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("Grenade");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;

}

void AProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	// Prevent colliding with the instigator
	if (GetOwner())
	{
		/*FString ActorName = Owner->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Owner name : %s"), *ActorName);*/
		CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	}

	//  Binding Delegate
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	//	输出引擎碰撞的物体名称 
	if (ImpactResult.GetActor() != nullptr)
	{
		FString ActorName = ImpactResult.GetActor()->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Grenade bounced off: %s"), *ActorName);
	}

	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

void AProjectileGrenade::Destroyed()
{
	ExplodeDamage();

	Super::Destroyed();
}