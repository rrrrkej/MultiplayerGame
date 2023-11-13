#pragma once

#define TRACE_LENGTH 8000

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "AssaultRifle"),	// mag = 30
	EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),	// mag = 1
	EWT_Pistol UMETA(Displayname = "Pistol"),	// mag = 12
	EWT_SubmachineGun UMETA(DisplayName = "Submachine Gun"), // mag = 28
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),	// mag = 6
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"), // mag = 4
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),	// mag = 4
	EWT_Flag UMETA(DisplayName = "Flag"), // Special pickup

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};