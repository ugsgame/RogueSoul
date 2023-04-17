// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "Widget/UIParticle.h"
#include "UIParticlePrivatePCH.h"
#include "Widget/UIParticle.h"

#define LOCTEXT_NAMESPACE "UMG"

UUIParticle::UUIParticle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUIParticle::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    ParticleCanvas.Reset();
}

TSharedRef<SWidget> UUIParticle::RebuildWidget()
{
    ParticleCanvas = SNew(SUIParticle);
    ParticleCanvas->SetOnParticleEnd(BIND_UOBJECT_DELEGATE(ParticleEndDelegate, HandleOnParticleEnd));
	if (bPlayParticle || (Asset && Asset->AutoPlay))
	{
		Play();
	}
    return ParticleCanvas.ToSharedRef();
}

void UUIParticle::HandleOnParticleEnd()
{
	IsPlaying = false;
	bPlayParticle = false;
    EventOnEnd.Broadcast();
    return;
}

void UUIParticle::Play()
{
    IsPlaying = true;
	bPlayParticle = true;
    if ( ParticleCanvas.IsValid() && Asset )
    {
        ParticleCanvas->Init(Asset);
        ParticleCanvas->Play();
    }
}

void UUIParticle::Stop()
{
	IsPlaying = false;
	bPlayParticle = false;
	if (ParticleCanvas.IsValid())
	{
		ParticleCanvas->Stop();
	}
}

void UUIParticle::StopEmit()
{
	if (ParticleCanvas.IsValid())
	{
		ParticleCanvas->StopEmit();
	}
}

void UUIParticle::SetPlayParticle(bool InPlayParticle)
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

const FText UUIParticle::GetPaletteCategory()
{
	return LOCTEXT("Particle", "Particle");
}

#endif

#undef LOCTEXT_NAMESPACE
