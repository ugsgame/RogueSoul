// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ILoadingScreenModule.h"
#include "LoadingScreenSettings.h"
#include "SStartupLoadingScreen.h"
#include "SDefaultLoadingScreen.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Images/SThrobber.h"
#include "Kismet/GameplayStatics.h"

// Jayou 2020-11-15
// ----------------------------
#include "ShaderPipelineCache.h"

DEFINE_LOG_CATEGORY_STATIC(ShaderPipelineCache, Log, All);
// ----------------------------

#define LOCTEXT_NAMESPACE "LoadingScreen"

class FLoadingScreenModule : public ILoadingScreenModule
{
public:
	FLoadingScreenModule();

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool IsGameModule() const override
	{
		return true;
	}

public:
	virtual void IsPlayingLoadingScreen(bool& bIsPlaying, bool& bIsStartup) override { bIsPlaying = bIsPlayingLoadingScreen; bIsStartup = bIsInStartupScreen; }
	virtual void StartupScreen_ShowLoadingComplete(FText LoadCompletePrompt) override;
	virtual void DefaultScreen_UpdateLoadingText(FText NewLoadingText) override;
	virtual void StopLoadingScreen() override;
	virtual void ShowDefaultLoadingScreen(bool bShow = false) override;

public:
	virtual void PlayGlobalBGM(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, USoundConcurrency* ConcurrencySettings = nullptr) override;
	virtual bool IsGlobalBGMPlaying() override;
	virtual void StopGlobalBGM(float FadeOutDuration, float FadeVolumeLevel, const EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear) override;

private:
	void HandlePrepareLoadingScreen();
	void HandleLoadingScreenStarted();
	void HandleLoadingScreenFinished();

	void BeginLoadingScreen(const FLoadingScreenDescription& ScreenDescription, const bool bIsStartup);

	bool bIsPlayingLoadingScreen;
	bool bIsInStartupScreen;

	bool bShowDefaultLoadingCreen;

	TSharedPtr<SStartupLoadingScreen> StartupLoadingScreenWidget;
	TSharedPtr<SDefaultLoadingScreen> DefaultLoadingScreenWidget;

	UAudioComponent* GlobalBGM_AC;
};

IMPLEMENT_MODULE(FLoadingScreenModule, LoadingScreen)

FLoadingScreenModule::FLoadingScreenModule()
{

}

void FLoadingScreenModule::StartupModule()
{
	if ( !IsRunningDedicatedServer() && FSlateApplication::IsInitialized() )
	{
		// Load for cooker reference
		const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
		for ( const FStringAssetReference& Ref : Settings->StartupScreen.Images )
		{
			Ref.TryLoad();
		}

		for ( const FStringAssetReference& Ref : Settings->DefaultScreen.Images )
		{
			Ref.TryLoad();
		}

		if ( IsMoviePlayerEnabled() )
		{
			GetMoviePlayer()->OnMoviePlaybackStarted().AddRaw(this, &FLoadingScreenModule::HandleLoadingScreenStarted);
			GetMoviePlayer()->OnPrepareLoadingScreen().AddRaw(this, &FLoadingScreenModule::HandlePrepareLoadingScreen);
			GetMoviePlayer()->OnMoviePlaybackFinished().AddRaw(this, &FLoadingScreenModule::HandleLoadingScreenFinished);
		}

		// Prepare the startup screen, the PrepareLoadingScreen callback won't be called
		// if we've already explictly setup the loading screen.
		BeginLoadingScreen(Settings->StartupScreen, true);
	}

	GlobalBGM_AC = 0;
}

void FLoadingScreenModule::ShutdownModule()
{
	if ( !IsRunningDedicatedServer() )
	{
		GetMoviePlayer()->OnPrepareLoadingScreen().RemoveAll(this);
		GetMoviePlayer()->OnMoviePlaybackStarted().RemoveAll(this);
		GetMoviePlayer()->OnMoviePlaybackFinished().RemoveAll(this);

		if (StartupLoadingScreenWidget.IsValid())
			StartupLoadingScreenWidget.Reset();

		if (DefaultLoadingScreenWidget.IsValid())
			DefaultLoadingScreenWidget.Reset();

		//Causes crash.
		//if (GlobalBGM_AC)
		//{
		//	GlobalBGM_AC->RemoveFromRoot();
		//	GlobalBGM_AC->DestroyComponent();
		//	GlobalBGM_AC = 0;
		//}
	}
}

void FLoadingScreenModule::HandlePrepareLoadingScreen()
{
	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
	BeginLoadingScreen(Settings->DefaultScreen, false);
}

void FLoadingScreenModule::HandleLoadingScreenStarted()
{
	// Jayou 2020-11-15
	// --------------------------------------------------------------------------------------------------
	FShaderPipelineCache::SetBatchMode(FShaderPipelineCache::BatchMode::Fast);
	FShaderPipelineCache::ResumeBatching();
	UE_LOG(ShaderPipelineCache, Log, TEXT("----------------------Begin PSO Cache----------------------"));
	// --------------------------------------------------------------------------------------------------

	bIsPlayingLoadingScreen = true;
}

void FLoadingScreenModule::HandleLoadingScreenFinished()
{
	// Jayou 2020-11-15
	// ------------------------------------------------------------------------------------------------------------
	while (true)
	{
		if (FShaderPipelineCache::NumPrecompilesRemaining() > 0)
		{
			//UE_LOG(ShaderPipelineCache, Log, TEXT("NumPrecompilesRemaining: %d"), FShaderPipelineCache::NumPrecompilesRemaining());
		}
		else
		{
			UE_LOG(ShaderPipelineCache, Log, TEXT("----------------------Finish PSO Cache----------------------"));
			break;
		}
	}
	// ------------------------------------------------------------------------------------------------------------

	bIsPlayingLoadingScreen = false;

	bShowDefaultLoadingCreen = false;

	if (StartupLoadingScreenWidget.IsValid())
		StartupLoadingScreenWidget.Reset();

	if (DefaultLoadingScreenWidget.IsValid())
		DefaultLoadingScreenWidget.Reset();
}

void FLoadingScreenModule::BeginLoadingScreen(const FLoadingScreenDescription& ScreenDescription, const bool bIsStartup)
{
	bIsInStartupScreen = bIsStartup;

	if (bIsStartup)
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.MinimumLoadingScreenDisplayTime = ScreenDescription.MinimumLoadingScreenDisplayTime;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = ScreenDescription.bAutoCompleteWhenLoadingCompletes;
		LoadingScreen.bMoviesAreSkippable = ScreenDescription.bMoviesAreSkippable;
		LoadingScreen.bWaitForManualStop = ScreenDescription.bWaitForManualStop;
		LoadingScreen.MoviePaths = ScreenDescription.MoviePaths;
		LoadingScreen.PlaybackType = ScreenDescription.PlaybackType;
		LoadingScreen.bAllowEngineTick = true;

		if (ScreenDescription.bShowUIOverlay)
		{
			LoadingScreen.WidgetLoadingScreen = SAssignNew(StartupLoadingScreenWidget, SStartupLoadingScreen, ScreenDescription);
		}
		
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
	else
	{
		if (bShowDefaultLoadingCreen)
		{
			FLoadingScreenAttributes LoadingScreen;
			LoadingScreen.MinimumLoadingScreenDisplayTime = ScreenDescription.MinimumLoadingScreenDisplayTime;
			LoadingScreen.bAutoCompleteWhenLoadingCompletes = ScreenDescription.bAutoCompleteWhenLoadingCompletes;
			LoadingScreen.bMoviesAreSkippable = ScreenDescription.bMoviesAreSkippable;
			LoadingScreen.bWaitForManualStop = ScreenDescription.bWaitForManualStop;
			LoadingScreen.MoviePaths = ScreenDescription.MoviePaths;
			LoadingScreen.PlaybackType = ScreenDescription.PlaybackType;
			LoadingScreen.bAllowEngineTick = true;

			if (ScreenDescription.bShowUIOverlay)
			{
				LoadingScreen.WidgetLoadingScreen = SAssignNew(DefaultLoadingScreenWidget, SDefaultLoadingScreen, ScreenDescription);
			}
			GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
		}
		else
		{
			bIsPlayingLoadingScreen = false;
		}
	}
	
	/*FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.MinimumLoadingScreenDisplayTime = ScreenDescription.MinimumLoadingScreenDisplayTime;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = ScreenDescription.bAutoCompleteWhenLoadingCompletes;
	LoadingScreen.bMoviesAreSkippable = ScreenDescription.bMoviesAreSkippable;
	LoadingScreen.bWaitForManualStop = ScreenDescription.bWaitForManualStop;
	LoadingScreen.MoviePaths = ScreenDescription.MoviePaths;
	LoadingScreen.PlaybackType = ScreenDescription.PlaybackType;
	LoadingScreen.bAllowEngineTick = true;
	
	if ( ScreenDescription.bShowUIOverlay && (bIsStartup || bShowDefaultLoadingCreen))
	{
		if (bIsStartup)
			LoadingScreen.WidgetLoadingScreen = SAssignNew(StartupLoadingScreenWidget, SStartupLoadingScreen, ScreenDescription);
		else
			LoadingScreen.WidgetLoadingScreen = SAssignNew(DefaultLoadingScreenWidget, SDefaultLoadingScreen, ScreenDescription);
	}

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);*/
}

void FLoadingScreenModule::StartupScreen_ShowLoadingComplete(FText LoadCompletePrompt)
{
	if (StartupLoadingScreenWidget.IsValid())
	{
		//TODO:Image LoadingScreen Can Not Auto Complete When Loading Completes,need to stop right now!!!
		//StartupLoadingScreenWidget->ShowLoadingComplete(LoadCompletePrompt);
		GetMoviePlayer()->StopMovie();
		//
	}

}

void FLoadingScreenModule::DefaultScreen_UpdateLoadingText(FText NewLoadingText)
{
	if (DefaultLoadingScreenWidget.IsValid())
	{
		DefaultLoadingScreenWidget->UpdateLoadingText(NewLoadingText);
	}
}

void FLoadingScreenModule::ShowDefaultLoadingScreen(bool bShow)
{
	bShowDefaultLoadingCreen = bShow;
	if (bShow)
	{
		bIsPlayingLoadingScreen = true;
	}
}

void FLoadingScreenModule::StopLoadingScreen()
{
	GetMoviePlayer()->StopMovie();
}

void FLoadingScreenModule::PlayGlobalBGM(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier, float StartTime, USoundConcurrency* ConcurrencySettings)
{
	if (!GlobalBGM_AC)
	{
		GlobalBGM_AC = UGameplayStatics::SpawnSound2D(WorldContextObject, Sound, VolumeMultiplier, PitchMultiplier, StartTime, ConcurrencySettings, true, false);
		GlobalBGM_AC->AddToRoot();
	}
	else
	{
		GlobalBGM_AC->SetSound(Sound);
		GlobalBGM_AC->SetVolumeMultiplier(VolumeMultiplier);
		GlobalBGM_AC->SetPitchMultiplier(PitchMultiplier);
		GlobalBGM_AC->ConcurrencySet.Add(ConcurrencySettings);
		GlobalBGM_AC->Play(StartTime);
	}	
}

bool FLoadingScreenModule::IsGlobalBGMPlaying()
{
	return (GlobalBGM_AC && GlobalBGM_AC->IsPlaying());
}

void FLoadingScreenModule::StopGlobalBGM(float FadeOutDuration, float FadeVolumeLevel, const EAudioFaderCurve FadeCurve)
{
	if (GlobalBGM_AC && GlobalBGM_AC->IsPlaying())
	{
		GlobalBGM_AC->FadeOut(FadeOutDuration, FadeVolumeLevel, FadeCurve);
	}
}

float PointSizeToSlateUnits(float PointSize)
{
	//FreeTypeConstants::HorizontalDPI = 96;
	const float SlateFreeTypeHorizontalResolutionDPI = 96.0f;
	const float FreeTypeNativeDPI = 72.0f;
	const float PixelSize = PointSize * (SlateFreeTypeHorizontalResolutionDPI / FreeTypeNativeDPI);

	return PixelSize;
}

#undef LOCTEXT_NAMESPACE
