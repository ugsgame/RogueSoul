// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Widgets/Images/SImage.h"
#include "Particle/EasyParticleState.h"
#include "Asset/UIParticleAsset.h"

DECLARE_DELEGATE(ParticleEndDelegate)

class UIPARTICLE_API  SUIParticle : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SUIParticle)
	{}
	SLATE_END_ARGS()

	SUIParticle()
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
    void Init(UUIParticleAsset* Asset);
    
	TSharedPtr<FEasyParticleGroupState, ESPMode::ThreadSafe> EmitterGroup;

    //TArray<TSharedPtr<FEasyParticleState, ESPMode::ThreadSafe>> Emitters;
    
    /** See OnMouseButtonDown event */
    void SetOnParticleEnd(ParticleEndDelegate EventHandler);
    
    //EndDelegate//
    ParticleEndDelegate ParticleEnd;


private:
	bool FirstTick;
};


