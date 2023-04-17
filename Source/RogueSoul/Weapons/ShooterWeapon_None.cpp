// Fill out your copyright notice in the Description page of Project Settings.

#include "RogueSoul.h"
#include "ShooterWeapon_None.h"

AShooterWeapon_None::AShooterWeapon_None(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

	WeaponConfig.bInfiniteAmmo = true;
	WeaponConfig.bInfiniteClip = true;
}


