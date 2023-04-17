// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Widgets/Images/SImage.h"
#include "Particle/EasyParticleState.h"
#include "Asset/UIParticleEmitterAsset.h"

DECLARE_DELEGATE(ParticleEmitterEndDelegate)

class UIPARTICLE_API  SUIParticleEmitter : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SUIParticleEmitter)
	{}
	SLATE_END_ARGS()

	SUIParticleEmitter()
	{
    	SetCanTick(false);
		bCanSupportFocus = false;
	}

	/**
	* Construct this widget
	*
	* @param	InArgs	The declaration data for this widget
	*/
	void Construct(const FArguments& InArgs) {
		FirstTick = true;
	}
public:
	bool isActivity;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;

public:
	int GetParticleCount();
	int GetParticleCountInPool();

	void Play();
	void Stop();
	void StopEmit();

	//isend//
	bool IsEnd();

	//Init//
	void InitWithAsset(UUIParticleEmitterAsset* DataAsset);

	TSharedPtr<FEasyParticleRootState, ESPMode::ThreadSafe> RootEmitter;

	/** See OnMouseButtonDown event */
	void SetOnParticleEnd(ParticleEmitterEndDelegate EventHandler);

	//EndDelegate//
	ParticleEmitterEndDelegate ParticleEnd;


private:
	bool FirstTick;
};


