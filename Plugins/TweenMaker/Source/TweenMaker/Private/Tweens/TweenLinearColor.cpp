// Copyright 2018 Francesco Desogus. All Rights Reserved.

#include "Tweens/TweenLinearColor.h"

#include "TweenFactory/Latent/TweenLinearColorLatentFactory.h"
#include "GameFramework/Actor.h"
#include "Utils/Utility.h"
#include "Utils/EaseEquations.h"
#include "Materials/MaterialInstanceDynamic.h"

UTweenLinearColor::UTweenLinearColor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    mParameterName = FName("");
    From = FLinearColor();
    To = FLinearColor();
    CurrentValue = FLinearColor();
    TweenLatentProxy = nullptr;
}

/*
 **************************************************************************
 * Methods used internally. Should not be called.
 **************************************************************************
 */


void UTweenLinearColor::Init(const FName &pParameterName, const FLinearColor &pFrom, const FLinearColor &pTo,
                             ETweenLinearColorType pTweenType, UTweenLinearColorLatentFactory *pTweenLatentProxy)
{
    mParameterName = pParameterName;
    From = pFrom;
    To = pTo;
    mTweenType = pTweenType;
    TweenLatentProxy = pTweenLatentProxy;

    CurrentValue = FLinearColor();


    FromVector = FVector(From.R, From.G, From.B);
    ToVector = FVector(To.R, To.G, To.B);
    CurrentValueVector = FVector(0, 0, 0);
}

// Comments on the code are in the class UTweenFloat because they're pretty similar
bool UTweenLinearColor::Update(float pDeltaTime, float pTimeScale)
{
    Super::Update(pDeltaTime, pTimeScale);

    bool hasEnded = false;

    mElapsedTime += pDeltaTime * pTimeScale * mTimeScaleAbsolute;

    hasEnded = mElapsedTime >= mTweenDuration;

    if(hasEnded)
    {
        CurrentValue = To;
    }
    else
    {
        // Computing the values for the RGB part and the alpha separately
		float alpha = 0.0f;

		if(mCurveFloat)
		{
			EaseEquations::ComputeCustomVectorEasing(mCurveFloat, mElapsedTime, FromVector, ToVector, mTweenDuration, CurrentValueVector);
			alpha = EaseEquations::ComputeCustomFloatEasing(mCurveFloat, mElapsedTime, From.A, To.A, mTweenDuration);
		}
		else
		{
			EaseEquations::ComputeVectorEasing(mEaseType, mElapsedTime, FromVector, ToVector, mTweenDuration, CurrentValueVector);
			alpha = EaseEquations::ComputeFloatEasing(mEaseType, mElapsedTime, From.A, To.A, mTweenDuration);
		}

        CurrentValue = FLinearColor(CurrentValueVector.X, CurrentValueVector.Y, CurrentValueVector.Z, alpha);
    }

    switch(mTargetType)
    {
        case ETweenTargetType::Material:
        {
            UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(mTargetObject);
            UpdateForMaterial(material);
            break;
        }
    }

    return hasEnded;
}

void UTweenLinearColor::UpdateForMaterial(UMaterialInstanceDynamic *pTarget)
{
    switch(mTweenType)
    {
        case ETweenLinearColorType::MaterialVectorTo:
        case ETweenLinearColorType::MaterialVectorFromTo:
        {
            pTarget->SetVectorParameterValue(mParameterName, CurrentValue);
            break;
        }
    }
}

void UTweenLinearColor::PrepareTween()
{
    Super::PrepareTween();

    switch(mTweenType)
    {
        case ETweenLinearColorType::MaterialVectorTo:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::Material:
                {
                    UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(mTargetObject);
                    material->GetVectorParameterValue(FMaterialParameterInfo(mParameterName), From);

                    FromVector = FVector(From.R, From.G, From.B);
                    ToVector = FVector(To.R, To.G, To.B);
                    break;
                }
            }

            break;
        }
    }
}

void UTweenLinearColor::PositionAt(bool pBeginning)
{
	// Don't bother doing aything if the Tween has not even ever started
	if(bHasPreparedTween)
	{
		// Start again at the from!
		CurrentValue = pBeginning ? From : To;

		switch(mTargetType)
		{
			case ETweenTargetType::Material:
			{
				UMaterialInstanceDynamic *material = Cast<UMaterialInstanceDynamic>(mTargetObject);
				UpdateForMaterial(material);
				break;
			}
		}
	}
}

void UTweenLinearColor::OnTweenPausedInternal()
{
    Super::OnTweenPausedInternal();
}

void UTweenLinearColor::OnTweenResumedInternal()
{
    Super::OnTweenResumedInternal();
}

void UTweenLinearColor::OnTweenDeletedInternal()
{
    Super::OnTweenDeletedInternal();
}

void UTweenLinearColor::Restart(bool pRestartFromEnd)
{
    Super::Restart(pRestartFromEnd);

    if(pRestartFromEnd)
    {
        Swap(From, To);
        Swap(FromVector, ToVector);
    }

    mElapsedTime = 0.0f;
}

void UTweenLinearColor::Invert(bool pShouldInvertElapsedTime)
{
    Super::Invert(pShouldInvertElapsedTime);

    Swap(From, To);
    Swap(FromVector, ToVector);
}

void UTweenLinearColor::BroadcastOnTweenStart()
{
    Super::BroadcastOnTweenStart();

    if(TweenLatentProxy != nullptr)
    {
        TweenLatentProxy->OnTweenStart.Broadcast(this);
    }
    else
    {
        if(OnTweenStart.IsBound())
        {
            OnTweenStart.Broadcast(this);
        }

        OnTweenStartDelegate.ExecuteIfBound(this);
    }
}

void UTweenLinearColor::BroadcastOnTweenUpdate()
{
    Super::BroadcastOnTweenUpdate();

    if(TweenLatentProxy != nullptr)
    {
        TweenLatentProxy->OnTweenUpdate.Broadcast(this);
    }
    else
    {
        if(OnTweenUpdate.IsBound())
        {
            OnTweenUpdate.Broadcast(this);
        }

        OnTweenUpdateDelegate.ExecuteIfBound(this);
    }
}

void UTweenLinearColor::BroadcastOnTweenEnd()
{
    Super::BroadcastOnTweenEnd();

    if(TweenLatentProxy != nullptr)
    {
        TweenLatentProxy->OnTweenEnd.Broadcast(this);
    }
    else
    {
        if(OnTweenEnd.IsBound())
        {
            OnTweenEnd.Broadcast(this);
        }

        OnTweenEndDelegate.ExecuteIfBound(this);
    }
}
