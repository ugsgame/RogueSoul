// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Components/AudioComponent.h"

/**
 * The public interface to this module
 */
class ILoadingScreenModule : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline ILoadingScreenModule& Get()
	{
		return FModuleManager::LoadModuleChecked< ILoadingScreenModule >("LoadingScreen");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "LoadingScreen" );
	}

	/*
		Is playing loading screen now?

		bIsPlaying: Is a loading screen playing now?
		bIsStartup: is this loading screen is the very first startup loading screen?
	*/
	virtual void IsPlayingLoadingScreen(bool& bIsPlaying, bool& bIsStartup) {}

	/*
		Show loading complete during the very first startup loading screen.
	*/
	virtual void StartupScreen_ShowLoadingComplete(FText LoadCompletePrompt) {}

	/*
		Update loading status text during default loading screen.
	*/
	virtual void DefaultScreen_UpdateLoadingText(FText NewLoadingText) {}

	/*
		Stop a loading screen manually.
		Only in case FLoadingScreenSettings::bWaitForManualStop=true.
	*/
	virtual void StopLoadingScreen() {}

	virtual void ShowDefaultLoadingScreen(bool bShow = false) {}

	/*
	*	Global BGM #################################################################################################################
	*/
public:
	virtual void PlayGlobalBGM(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, USoundConcurrency* ConcurrencySettings = nullptr) {}
	virtual bool IsGlobalBGMPlaying() { return false; }
	virtual void StopGlobalBGM(float FadeOutDuration, float FadeVolumeLevel, const EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear) {}
	//##############################################################################################################################
};
