// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "MP_PlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

class AMP_HUD;
class UCharacterOverlay;
class AMP_GameMode;
class UInputMappingContext;
class UInputAction;
class UUserWidget;
/**
 * 
 */
UCLASS()
class MULTIPLAYERTPS_API AMP_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShidle);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 GrenadesNum);
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual  float GetServerTime(); //Synced with server world clock

	void OnMatchStateSet(FName State);
	// call the function corresponding to the value of MatchState
	void HandleMatchHasStarted();
	void HandleCooldown();

	// half time of RTT
	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	/**
	* Sync time between client and server
	*/

	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientRePortServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // Difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaTime);

	void PollInit();

	// retrieve the current MatchState
	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	// Play WidgetAnimation as high ping warning
	void HighPingWarning();
	void StopHighPingWarning();

	// Check Ping in tick()
	void CheckPing(float DeltaTime);

private:
	UPROPERTY()
	AMP_HUD* MP_HUD;

	UPROPERTY()
	AMP_GameMode* MP_GameMode;

	// Timer from GameMode class
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0; // As a standard for the countdown update.

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	/*
	* Initialize value settings
	*/
	float HUDHealth;
	bool bInitializeHealth = false;

	float HUDMaxHealth;
	float HUDShield;
	bool bInitializeShield = false;

	float HUDMaxShield;
	float HUDScore;
	bool bInitializeScore = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	int32 HUDGrenades;
	bool bInitializeGrenades = false;
	
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
	
	/*
	* HighPing settings
	*/
	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 4.f;
	
	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 5.f;

	// Check if high Ping, disable Weapon SSR
	UFUNCTION(Server, Reliable)
	void ServerReportPintStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
#pragma region UI
	/**
	* Return to main menu
	*/
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<UUserWidget> ReturnToMainMenuClass;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;
#pragma endregion
#pragma region input
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PlayerControllerMapping;

	/* InputAction */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReturnMenuAction;

protected:
	/* Set Mapping context and bind input action*/
	virtual void SetupInputComponent() override;

	/** Founction Bind to ReturnMenuAction */
	void ReturnMenu_Pressed(const FInputActionValue& Value);

#pragma endregion
};
