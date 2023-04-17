#include "LoadingScreenBPLibrary.h"
#include "ILoadingScreenModule.h"

void ULoadingScreenBPLibrary::IsPlayingLoadingScreen(bool& bIsPlaying, bool& bIsStartup)
{
	ILoadingScreenModule::Get().IsPlayingLoadingScreen(bIsPlaying, bIsStartup);
}

void ULoadingScreenBPLibrary::StopLoadingScreen()
{
	ILoadingScreenModule::Get().StopLoadingScreen();
}

void ULoadingScreenBPLibrary::PlayGlobalBGM(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier, float StartTime, USoundConcurrency* ConcurrencySettings)
{
	ILoadingScreenModule::Get().PlayGlobalBGM(WorldContextObject, Sound, VolumeMultiplier, PitchMultiplier, StartTime, ConcurrencySettings);
}

bool ULoadingScreenBPLibrary::IsGlobalBGMPlaying()
{
	return ILoadingScreenModule::Get().IsGlobalBGMPlaying();
}

void ULoadingScreenBPLibrary::StopGlobalBGM(float FadeOutDuration, float FadeVolumeLevel, const EAudioFaderCurve FadeCurve)
{
	ILoadingScreenModule::Get().StopGlobalBGM(FadeOutDuration, FadeVolumeLevel, FadeCurve);
}
