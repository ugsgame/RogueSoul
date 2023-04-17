#pragma once

#include "Components/AudioComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoadingScreenBPLibrary.generated.h"

UCLASS()
class ULoadingScreenBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "LoadingScreenBlueprintLibrary")
	void IsPlayingLoadingScreen(bool& bIsPlaying, bool& bIsStartup);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "LoadingScreenBlueprintLibrary")
	static void StopLoadingScreen();

public:
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "LoadingScreenBlueprintLibrary", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "2", UnsafeDuringActorConstruction = "true"))
	static void PlayGlobalBGM(const UObject* WorldContextObject, USoundBase* Sound, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, USoundConcurrency* ConcurrencySettings = nullptr);

	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "LoadingScreenBlueprintLibrary")
	static bool IsGlobalBGMPlaying();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "LoadingScreenBlueprintLibrary")
	static void StopGlobalBGM(float FadeOutDuration, float FadeVolumeLevel, const EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear);
};
