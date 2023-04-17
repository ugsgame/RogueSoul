// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "Widget/SUIParticleEmitter.h"
#include "UIParticlePrivatePCH.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widget/SUIParticleEmitter.h"
#include "Utility/UIParticleUtility.h"

void SUIParticleEmitter::InitWithAsset(UUIParticleEmitterAsset* DataAsset)
{
	isActivity = false;
	if (DataAsset)
	{
		if (!RootEmitter.IsValid())
		{
			RootEmitter = MakeShareable(new FEasyParticleRootState());
			FUIParticleFactory::Instance()->RefRoot(RootEmitter);
			RootEmitter->InitRoot(DataAsset, 0);
		}
		else
		{
			RootEmitter->InitRoot(DataAsset, 0);
		}
	}
}

void SUIParticleEmitter::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
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
	if (RootEmitter.IsValid())
	{
		if (RootEmitter->FirstRootTick)
		{
			RootEmitter->FirstTick(InDeltaTime, AllottedGeometry);
		}
		FUIParticleFactory::Instance()->TickRoot(RootEmitter.Get(), InDeltaTime, AllottedGeometry);
		RootEmitter->TrySpawnParticle(AllottedGeometry);
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

int32 SUIParticleEmitter::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (RootEmitter.IsValid())
	{
		FUIParticleFactory::Instance()->OnRootPaint(RootEmitter.Get(), Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}
	return LayerId;
}

int SUIParticleEmitter::GetParticleCount()
{
	if (RootEmitter.IsValid())
	{
		return RootEmitter->GetChildrenParticleCount();
	}
	return 0;
}
int SUIParticleEmitter::GetParticleCountInPool()
{
	if (RootEmitter.IsValid())
	{
		return RootEmitter->GetChildrenParticleCountInPool();
	}
	return 0;
}


void SUIParticleEmitter::Play()
{
	if (RootEmitter.IsValid())
	{
		RootEmitter->ResetRoot();
    	SetCanTick(true);
		isActivity = true;
	}
}

void SUIParticleEmitter::Stop()
{
	if (RootEmitter.IsValid())
	{
		RootEmitter->ResetRoot();
	}
    SetCanTick(false);
	isActivity = false;
}

void SUIParticleEmitter::StopEmit()
{
	if (RootEmitter.IsValid())
	{
		RootEmitter->StopEmit();
	}
}

bool SUIParticleEmitter::IsEnd()
{
	if (RootEmitter.IsValid())
	{
		return RootEmitter->IsEmitterEnd() && RootEmitter->IsAllChildrenEnd();
	}
	else
	{
		return true;
	}
}


void SUIParticleEmitter::SetOnParticleEnd(ParticleEndDelegate EventHandler)
{
	ParticleEnd = EventHandler;
}


FVector2D SUIParticleEmitter::ComputeDesiredSize(float) const
{
	return FVector2D::ZeroVector;
}
