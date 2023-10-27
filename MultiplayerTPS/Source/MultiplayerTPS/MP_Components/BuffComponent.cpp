// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "MultiplayerTPS/Character/MP_Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	//bReplicates = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || !Character->GetCharacterMovement()) return;

	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeed,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
	
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::ResetJump()
{
	if (Character == nullptr || !Character->GetCharacterMovement()) return;

	Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	MulticastJumpBuff(InitialJumpVelocity);
	
	
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumppVelocity)
{
	if (Character == nullptr || !Character->GetCharacterMovement()) return;

	Character->GetCharacterMovement()->JumpZVelocity = JumppVelocity;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ShieldReplenishTime)
{
	bReplenishShield = true;
	ShieldReplenishRate = ShieldAmount / ShieldReplenishTime;
	AmountToShieldReplenish += ShieldAmount;
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishShield || Character == nullptr || Character->IsElimmed()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	AmountToShieldReplenish -= ReplenishThisFrame;

	if (AmountToShieldReplenish <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishShield = false;
		AmountToShieldReplenish = 0.f;
	}
}