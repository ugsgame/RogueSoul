// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "RogueSoul.h"
#include "Player/ShooterPlayerController_Menu.h"
#include "ShooterStyle.h"


AShooterPlayerController_Menu::AShooterPlayerController_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void AShooterPlayerController_Menu::PostInitializeComponents() 
{
	Super::PostInitializeComponents();

	FShooterStyle::Initialize();
}
