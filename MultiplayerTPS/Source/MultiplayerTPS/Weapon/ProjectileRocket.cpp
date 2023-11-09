// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "MultiplayerTPS/DebugHeader.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("RocketMesh");
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	// 生成火箭弹的弹道粒子
	SpawnTrailSystem();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}
	AMP_Character* OwnerCharacter = Cast<AMP_Character>(GetOwner());
	AMP_PlayerController* OwnerController = Cast<AMP_PlayerController>(OwnerCharacter->Controller);

	if(OwnerController && OwnerCharacter)
	{
		if (!bUseServerSideRewind)
		{ 
			if (OwnerCharacter->HasAuthority())  // 不使用SSR或者服务器本机则直接服务端计算伤害
			{
				ExplodeDamage();
			}
		}
		else if(bUseServerSideRewind && OwnerController->IsLocalController()) // 使用SSR请求，以本地为主。
		{
			if (OwnerCharacter->HasAuthority())	// 服务器不使用SSR
			{
				ExplodeDamage();
			}
			else //	客户端本地执行ssr
			{
				// 获取爆炸范围内的Character
				TArray<AActor*> HitCharacters;	// 存放范围内的Characters
				TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes; // 碰撞检测参数，看不懂
				ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

				UKismetSystemLibrary::SphereOverlapActors(
					GetWorld(),					// 上下文相关世界
					GetActorLocation(),			// 判定坐标
					DamageOuterRaius,			// 判定范围
					ObjectTypes,				// 碰撞通道过滤结果
					AActor::StaticClass(),		// 无视特定类
					TArray<AActor*>(),			// 无视特定对象
					HitCharacters				// 存储结果
				);
				for (AActor* Actor : HitCharacters)
				{
					DebugHeader::Print(FString::Printf(TEXT("ActorName: %s"), *Actor->GetName()), FColor::Blue);
				}
				// 发送SSR请求
			}
		}
	}
	
	StartDestroyTimer();

	// Play hit effects
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	// Disable physical properties after damage
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

#if WITH_EDITOR
void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	//FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (Event.GetPropertyName() == GET_MEMBER_NAME_CHECKED(AProjectileRocket, InitialSpeed))
	{
		if (RocketMovementComponent)
		{
			RocketMovementComponent->InitialSpeed = InitialSpeed;
			RocketMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif


void AProjectileRocket::Destroyed()
{
	//NULL, Used to avoid Supper::Destroyed()
}
