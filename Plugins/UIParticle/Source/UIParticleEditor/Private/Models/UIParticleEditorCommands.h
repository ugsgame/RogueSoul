// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "UIParticleEditorStyle.h"

class FUIParticleEditorCommands : public TCommands<FUIParticleEditorCommands>
{
public:

	FUIParticleEditorCommands()
		: TCommands<FUIParticleEditorCommands>(TEXT("UIParticleEditor"), NSLOCTEXT("Contexts", "UIParticleEditor", "UIParticleEditor Plugin"), NAME_None, FUIParticleEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:

	TSharedPtr< FUICommandInfo > BeginPlay;
	TSharedPtr< FUICommandInfo > Stop;
	TSharedPtr< FUICommandInfo > StopEmit;
	TSharedPtr< FUICommandInfo > CaptureThumbnail;
	TSharedPtr< FUICommandInfo > ClearThumbnail;

};

class FUIParticleEmitterEditorCommands : public TCommands<FUIParticleEmitterEditorCommands>
{
public:

	FUIParticleEmitterEditorCommands()
		: TCommands<FUIParticleEmitterEditorCommands>(TEXT("UIParticleEmitterEditor"), NSLOCTEXT("Contexts", "UIParticleEmitterEditor", "UIParticleEmitterEditor Plugin"), NAME_None, FUIParticleEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:

	TSharedPtr< FUICommandInfo > BeginPlay_Emitter;
	TSharedPtr< FUICommandInfo > Stop_Emitter;
	TSharedPtr< FUICommandInfo > StopEmit_Emitter;
	TSharedPtr< FUICommandInfo > CaptureThumbnail_Emitter;
	TSharedPtr< FUICommandInfo > ClearThumbnail_Emitter;
};
