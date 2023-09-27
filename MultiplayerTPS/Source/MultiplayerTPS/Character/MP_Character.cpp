// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "../DebugHeader.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerTPS/Weapon/Weapon.h"
#include "MultiplayerTPS/MP_Components/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerTPS/Character/MP_AnimInstance.h"
#include "MultiplayerTPS/MultiplayerTPS.h"
#include "MultiplayerTPS/PlayerController/MP_PlayerController.h"
#include "MultiplayerTPS/GameMode/MP_GameMode.h"
#include "Timermanager.h"

// Sets default values
AMP_Character::AMP_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//initialize CameraBoom attribute
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	//initialize FollowCamera attribute
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	//initialize overhead WidgetComponent
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	//initialize CombatComponent
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	//Set properties in CMC
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 720.f);

	//Set Camera block
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

	//Set Network properties
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	//initial properties
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AMP_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 在AMP_Character的生命周期内，如果OverlappingWeapon的值发生了变化，服务器会发送到每一个客户端
	//DOREPLIFETIME(AMP_Character, OverlappingWeapon);
	// 条件发送
	DOREPLIFETIME_CONDITION(AMP_Character, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AMP_Character, Health);
}

void AMP_Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void AMP_Character::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimuProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AMP_Character::Elim()
{
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer, 
		this, 
		&AMP_Character::ElimTimerFinished, 
		ElimDelay
		);
}

void AMP_Character::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
}

void AMP_Character::ElimTimerFinished()
{
	AMP_GameMode* MP_GameMode = GetWorld()->GetAuthGameMode<AMP_GameMode>();
	if (MP_GameMode)
	{
		MP_GameMode->RequestRespawn(this, Controller);
	}
}

// Called every frame
void AMP_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}

	HideCamera();
}

// Called when the game starts or when spawned
void AMP_Character::BeginPlay()
{
	Super::BeginPlay();
	
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Initialzied properties in HUD
	UpdateHUDHealth();

	// Bind event in server
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMP_Character::ReceiveDamage);
	}
}

void AMP_Character::AimOffset(float DeltaTime)
{
	if(CombatComponent && CombatComponent->EquippedWeapon == nullptr)  return;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)	//standing still, not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(StartingAimRotation, CurrentAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;	
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir)	// running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true; 
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

float AMP_Character::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AMP_Character::OnRep_Health()
{
	PlayHitReactMontage();
	UpdateHUDHealth();
}

void AMP_Character::UpdateHUDHealth()
{
	MP_PlayerController = MP_PlayerController == nullptr ? Cast<AMP_PlayerController>(Controller) : MP_PlayerController;
	if (MP_PlayerController)
	{
		MP_PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AMP_Character::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	//由于Pitch在网络传输的过程中由有符号数变成无符号数，所以要映射回去
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AMP_Character::SimuProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void AMP_Character::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f)
	{
		AMP_GameMode* MP_GameMode = GetWorld()->GetAuthGameMode<AMP_GameMode>();
		if (MP_GameMode)
		{
			MP_PlayerController = MP_PlayerController == nullptr ? Cast<AMP_PlayerController>(Controller) : MP_PlayerController;
			AMP_PlayerController* AttackerController = Cast<AMP_PlayerController>(InstigatorController);
			MP_GameMode->PlayerEliminated(this, MP_PlayerController, AttackerController);
		}
	}
	
}

void AMP_Character::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	 }
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AMP_Character::HideCamera()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}

	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AMP_Character::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleIronsight") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMP_Character::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMP_Character::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

//called by server
void AMP_Character::SetOverlappingWeapon(AWeapon* Weapon)
{
	//非严谨条件，用于出Weapon范围的时候关掉PickupWidget
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;

	//检查是否是服务器端，如果是客户端的话返回false
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AMP_Character::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AMP_Character::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

//Get the variable EquippedWeapon in CombatComponent class
bool AMP_Character::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

//Get the variable bAiming in CombatComponent class
bool AMP_Character::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

AWeapon* AMP_Character::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	return CombatComponent->EquippedWeapon;
}

FVector AMP_Character::GetHitTarget() const
{

	if (CombatComponent == nullptr) return FVector();
	return CombatComponent->HitTarget;
}

#pragma region Public Getter and Setter



#pragma endregion

#pragma region InputBinding
// Called to bind functionality to input
void AMP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMP_Character::Jump);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMP_Character::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMP_Character::Look);

		//Interaction
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AMP_Character::Interaction);

		//Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AMP_Character::Crouch);

		//Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AMP_Character::Aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AMP_Character::EndAim);

		//Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AMP_Character::Fire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AMP_Character::EndFire);
	}
}

void AMP_Character::Move(const FInputActionValue& Value)
{
	// input 2D Value
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out controller forward direction
		const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);

		// get forward vector
		const FVector ForWardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		//add movement
		AddMovementInput(ForWardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMP_Character::Look(const FInputActionValue& Value)
{
	// input 2D Value
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void AMP_Character::Jump(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		Super::UnCrouch();
	}
	else
	{
		Super::Jump();
	}
	
}

//Called by server
void AMP_Character::Interaction(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void AMP_Character::Crouch(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		Super::UnCrouch();
	}
	else
	{
		Super::Crouch();
	}
	
}

void AMP_Character::Aim(const FInputActionValue& Value)
{
	bool bAiming = Value.Get<bool>();
	if (CombatComponent)
	{
		CombatComponent->SetAiming(bAiming);
	}
	
}

void AMP_Character::EndAim(const FInputActionValue& Value)
{
	bool bAiming = Value.Get<bool>();
	if (CombatComponent)
	{
		CombatComponent->SetAiming(bAiming);
	}
}

void AMP_Character::Fire(const FInputActionValue& Value)
{
	bool bFire = Value.Get<bool>();
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->FireButtonpressed(bFire);
	}
}

void AMP_Character::EndFire(const FInputActionValue& Value)
{
	bool bFire = Value.Get<bool>();
	if (CombatComponent)
	{
		CombatComponent->FireButtonpressed(bFire);
	}
}

#pragma endregion
