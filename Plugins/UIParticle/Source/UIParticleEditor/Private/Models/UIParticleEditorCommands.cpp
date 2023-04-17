// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleEditorCommands.h"
#include "UIParticleEditorPrivatePCH.h"

#define LOCTEXT_NAMESPACE "FUIParticleEditorModule"

void FUIParticleEditorCommands::RegisterCommands()
{
	UI_COMMAND(BeginPlay, "Play", "PlayUIParticle", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar));
	UI_COMMAND(Stop, "Stop", "StopUIParticle", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StopEmit, "StopEmit", "StopEmitUIParticle", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(CaptureThumbnail, "CaptureThumbnail", "CaptureThumbnail", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(ClearThumbnail, "ClearThumbnail", "ClearThumbnail", EUserInterfaceActionType::Button, FInputGesture());
}

void FUIParticleEmitterEditorCommands::RegisterCommands()
{
	UI_COMMAND(BeginPlay_Emitter, "Play", "PlayUIParticleEmitter", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar));
	UI_COMMAND(Stop_Emitter, "Stop", "StopUIParticleEmitter", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(StopEmit_Emitter, "StopEmit", "StopEmitUIParticleEmitter", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(CaptureThumbnail_Emitter, "CaptureThumbnail", "CaptureThumbnailEmitter", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(ClearThumbnail_Emitter, "ClearThumbnail", "ClearThumbnailEmitter", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
