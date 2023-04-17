// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "Widget/SUIParticle.h"
#include "UIParticlePrivatePCH.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widget/UIParticle.h"
#include "Utility/UIParticleUtility.h"


void SUIParticle::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_UIParticleTickTime);
    SLeafWidget::Tick(AllottedGeometry,InCurrentTime,InDeltaTime);
    
	if (FirstTick)
	{
		FirstTick = false;
		return;
	}
    if (isActivity==false)
    {
        SetCanTick(false);
    }

	if (EmitterGroup.IsValid())
	{
		if (EmitterGroup->FirstRootTick)
		{
			EmitterGroup->FirstTick(InDeltaTime, AllottedGeometry);
		}

		EmitterGroup->Tick(InDeltaTime, AllottedGeometry);
		EmitterGroup->TrySpawnParticle(AllottedGeometry);
	}
    
    if (IsEnd())
    {
        SetCanTick(false);
        isActivity = false;
        if (ParticleEnd.IsBound())
        {
            ParticleEnd.Execute();
        }
    }
}

int32 SUIParticle::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (EmitterGroup.IsValid())
	{
		EmitterGroup->OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}
    return LayerId;
}


int SUIParticle::GetParticleCount()
{
	if (EmitterGroup.IsValid())
	{
		return EmitterGroup->GetParticleCount();
	}
	return 0;
}

int SUIParticle::GetParticleCountInPool()
{
	if (EmitterGroup.IsValid())
	{
		return EmitterGroup->GetParticleCountInPool();
	}
	return 0;
}

void SUIParticle::Play()
{
	if (EmitterGroup.IsValid())
	{
		if (EmitterGroup->Play())
		{
			SetCanTick(true);
			isActivity = true;
		}
	}
}

void SUIParticle::Stop()
{
	if (EmitterGroup.IsValid())
	{
		EmitterGroup->Stop();
	}
    SetCanTick(false);
	isActivity = false;
}

void SUIParticle::StopEmit()
{
	if (EmitterGroup.IsValid())
	{
		EmitterGroup->StopEmit();
	}
}

bool SUIParticle::IsEnd()
{
	if (EmitterGroup.IsValid())
	{
		return EmitterGroup->IsEnd();
	}
	return true;
}


void SUIParticle::SetOnParticleEnd(ParticleEndDelegate EventHandler)
{
    ParticleEnd = EventHandler;
}

void SUIParticle::Init(UUIParticleAsset* Asset)
{
    isActivity = false;
	if (Asset)
	{
		if (!EmitterGroup.IsValid())
		{
			EmitterGroup = MakeShareable(new FEasyParticleGroupState());
			EmitterGroup->InitWithAsset(Asset);
		}
		else
		{
			EmitterGroup->InitWithAsset(Asset);
		}
	}
}

FVector2D SUIParticle::ComputeDesiredSize(float) const
{
    return FVector2D::ZeroVector;
}
