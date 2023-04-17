// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "RogueSoul.h"
#include "ShooterGameDelegates.h"

#include "ShooterMenuSoundsWidgetStyle.h"
#include "ShooterMenuWidgetStyle.h"
#include "ShooterMenuItemWidgetStyle.h"
#include "ShooterOptionsWidgetStyle.h"
#include "ShooterScoreboardWidgetStyle.h"
#include "ShooterChatWidgetStyle.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"



#include "UI/Style/ShooterStyle.h"


class FRogueSoulModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		InitializeShooterGameDelegates();
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		//Hot reload hack
		FSlateStyleRegistry::UnRegisterSlateStyle(FShooterStyle::GetStyleSetName());
		FShooterStyle::Initialize();
	}

	virtual void ShutdownModule() override
	{
		FShooterStyle::Shutdown();
	}
};

IMPLEMENT_GAME_MODULE(FRogueSoulModule, RogueSoul);

DEFINE_LOG_CATEGORY(LogShooter)
DEFINE_LOG_CATEGORY(LogShooterWeapon)
