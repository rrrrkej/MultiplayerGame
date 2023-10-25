// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

class AMP_Character;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERTPS_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class AMP_Character;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealingTime);
	void ReplenishShield(float ShieldAmount, float ShieldReplenishTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);

	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpVelocity(float Velocity);
protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

private:
	UPROPERTY()
	AMP_Character* Character;

	/*
	* Health
	*/
	bool bHealing = false;
	float HealingRate = 0.f;	// healing per second
	float AmountToHeal = 0.f;	// record amount of healing needed

	/*
	* Shield
	*/
	bool bReplenishShield = false;
	float ShieldReplenishRate = 0.f;	//	Replenish shield per second
	float AmountToShieldReplenish = 0.f;			//	total of shield replenish 

	/*	
	* Speed Buff
	*/
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;		//	Character initial speed
	float InitialCrouchSpeed;	//	Character initial crouch speed

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/*
	* Jump Buff
	*/
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumppVelocity);
};

