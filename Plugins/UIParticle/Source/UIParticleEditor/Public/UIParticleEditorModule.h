// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "IAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

class FToolBarBuilder;
class FMenuBuilder;

class FUIParticleEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	/** All created asset type actions.  Cached here so that we can unregister them during shutdown. */
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;
    
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		CreatedAssetTypeActions.Add(Action);
	}
};
