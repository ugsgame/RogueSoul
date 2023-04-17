// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Components/Widget.h"
#include "Widget/SUIParticleEmitter.h"
#include "UIParticleEmitter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEasyParticleEndEvent);

UCLASS()
class UIPARTICLE_API  UUIParticleEmitter : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	/** Image to draw */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle")
        UUIParticleEmitterAsset* Asset;
	
	UPROPERTY(BlueprintAssignable, Category = "Particle")
		FOnEasyParticleEndEvent EventOnEnd;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particle)
		uint8 bPlayParticle : 1;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Particle" )
		bool IsPlaying;

	UFUNCTION(BlueprintCallable, Category = Particle)
		void Play();

	UFUNCTION(BlueprintCallable, Category = Particle)
		void Stop();

	UFUNCTION(BlueprintCallable, Category = Particle)
		void StopEmit();

	UFUNCTION(BlueprintCallable, Category = Particle)
		void SetPlayParticle(bool InPlayParticle);

public:
	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface
#if WITH_EDITOR
	const FText GetPaletteCategory();
#endif

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface

	void HandleOnParticleEnd();

protected:
	TSharedPtr<SUIParticleEmitter> ParticleCanvas;
};
