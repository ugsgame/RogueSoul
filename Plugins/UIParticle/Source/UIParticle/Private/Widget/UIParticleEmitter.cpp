// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "Widget/UIParticleEmitter.h"
#include "UIParticlePrivatePCH.h"
#include "Widget/UIParticleEmitter.h"

#define LOCTEXT_NAMESPACE "UMG"

UUIParticleEmitter::UUIParticleEmitter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUIParticleEmitter::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	ParticleCanvas.Reset();
}

TSharedRef<SWidget> UUIParticleEmitter::RebuildWidget()
{
	ParticleCanvas = SNew(SUIParticleEmitter);
	ParticleCanvas->SetOnParticleEnd(BIND_UOBJECT_DELEGATE(ParticleEmitterEndDelegate, HandleOnParticleEnd));
    if (bPlayParticle || ( Asset && Asset->AutoPlay ))
	{
		Play();
	}
	return ParticleCanvas.ToSharedRef();
}

void UUIParticleEmitter::HandleOnParticleEnd()
{
	IsPlaying = false;
	bPlayParticle = false;
	EventOnEnd.Broadcast();
	return;
}

void UUIParticleEmitter::Play()
{
	IsPlaying = true;
	bPlayParticle = true;
	if (ParticleCanvas.IsValid() && Asset)
	{
		ParticleCanvas->InitWithAsset(Asset);
		ParticleCanvas->Play();
	}
}

void UUIParticleEmitter::Stop()
{
	IsPlaying = false;
	bPlayParticle = false;
	if (ParticleCanvas.IsValid())
	{
		ParticleCanvas->Stop();
	}
}

void UUIParticleEmitter::StopEmit()
{
	if (ParticleCanvas.IsValid())
	{
		ParticleCanvas->StopEmit();
	}
}

void UUIParticleEmitter::SetPlayParticle(bool InPlayParticle)
{
	if (bPlayParticle != InPlayParticle)
	{
		bPlayParticle = InPlayParticle;
		if (bPlayParticle)
		{
			Play();
		}
		else
		{
			StopEmit();
		}
	}
}

#if WITH_EDITOR

const FText UUIParticleEmitter::GetPaletteCategory()
{
	return LOCTEXT("Particle", "Particle");
}

#endif

#undef LOCTEXT_NAMESPACE
