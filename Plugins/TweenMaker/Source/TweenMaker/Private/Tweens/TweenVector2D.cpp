// Copyright 2018 Francesco Desogus. All Rights Reserved.

#include "Tweens/TweenVector2D.h"
#include "TweenFactory/Latent/TweenVector2DLatentFactory.h"
#include "GameFramework/Actor.h"
#include "Utils/Utility.h"
#include "Components/Widget.h"
#include "Utils/EaseEquations.h"

UTweenVector2D::UTweenVector2D(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    mFrom = FVector2D(0, 0);
    mTo = FVector2D(0, 0);
    mCurrentValue = FVector2D(0, 0);
    mTweenLatentProxy = nullptr;
}

/*
 **************************************************************************
 * Methods used internally. Should not be called.
 **************************************************************************
 */

void UTweenVector2D::Init(const FVector2D &pFrom, const FVector2D &pTo, ETweenVector2DType pTweenType, UTweenVector2DLatentFactory *pTweenLatentProxy)
{
    mFrom = pFrom;
    mTo = pTo;
    mTweenType = pTweenType;
    mTweenLatentProxy = pTweenLatentProxy;

    mCurrentValue = FVector2D(0, 0);
    mBaseOffset = FVector2D(0, 0);
}

// Comments on the code are in the class UTweenFloat because they're pretty similar
bool UTweenVector2D::Update(float pDeltaTime, float pTimeScale)
{
    Super::Update(pDeltaTime, pTimeScale);

    bool hasEnded = false;

    mElapsedTime += pDeltaTime * pTimeScale * mTimeScaleAbsolute;

    hasEnded = mElapsedTime >= mTweenDuration;

    if(hasEnded)
    {
        mCurrentValue = mTo;
    }
    else
    {
		if(mCurveFloat)
		{
			EaseEquations::ComputeCustomVector2DEasing(mCurveFloat, mElapsedTime, mFrom, mTo, mTweenDuration, mCurrentValue);
		}
		else
		{
			EaseEquations::ComputeVector2DEasing(mEaseType, mElapsedTime, mFrom, mTo, mTweenDuration, mCurrentValue);
		}
    }

    switch(mTargetType)
    {
        case ETweenTargetType::UMG:
        {
            UWidget *widget = Cast<UWidget>(mTargetObject);
            UpdateForWidget(widget);
            break;
        }
        case ETweenTargetType::Custom:
        {
            break;
        }
    }

    return hasEnded;
}

/*
 **************************************************************************
 * Helper methods for updating individual classes
 **************************************************************************
 */

void UTweenVector2D::UpdateForWidget(UWidget *pTarget)
{
    switch(mTweenType)
    {
        case ETweenVector2DType::MoveTo:
        {
            pTarget->SetRenderTranslation(mCurrentValue);
            break;
        }
        case ETweenVector2DType::MoveBy:
        {
            pTarget->SetRenderTranslation(mCurrentValue + mBaseOffset);
            break;
        }
        case ETweenVector2DType::ScaleTo:
        {
            pTarget->SetRenderScale(mCurrentValue);
            break;
        }
        case ETweenVector2DType::ScaleBy:
        {
            pTarget->SetRenderScale(mCurrentValue + mBaseOffset);
            break;
        }
        case ETweenVector2DType::ShearTo:
        {
            pTarget->SetRenderShear(mCurrentValue);
            break;
        }
        // Does nothing
        case ETweenVector2DType::Custom:
        {
            break;
        }
    }
}

/*
 **************************************************************************
 * Overridden from UBaseTween
 **************************************************************************
 */

void UTweenVector2D::PrepareTween()
{
    Super::PrepareTween();

    switch(mTweenType)
    {
        case ETweenVector2DType::MoveTo:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::UMG:
                {
                    UWidget *widget = Cast<UWidget>(mTargetObject);
                    mFrom = widget->RenderTransform.Translation;
                    break;
                }
            }

            break;
        }
        case ETweenVector2DType::MoveBy:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::UMG:
                {
                    UWidget *widget = Cast<UWidget>(mTargetObject);
                    mFrom = FVector2D(0, 0);
                    mBaseOffset = widget->RenderTransform.Translation;;
                    break;
                }
            }

            break;
        }
        case ETweenVector2DType::ScaleTo:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::UMG:
                {
                    UWidget *widget = Cast<UWidget>(mTargetObject);
                    mFrom = widget->RenderTransform.Scale;
                    break;
                }
            }

            break;
        }
        case ETweenVector2DType::ScaleBy:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::UMG:
                {
                    UWidget *widget = Cast<UWidget>(mTargetObject);
                    mFrom = FVector2D(0, 0);
                    mBaseOffset = widget->RenderTransform.Scale;
                    break;
                }
            }

            break;
        }
        case ETweenVector2DType::ShearTo:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::UMG:
                {
                    UWidget *widget = Cast<UWidget>(mTargetObject);
                    mFrom = widget->RenderTransform.Shear;
                    break;
                }
            }

            break;
        }
        // Does nothing
        case ETweenVector2DType::Custom:
        {
            break;
        }
    }

}

void UTweenVector2D::PositionAt(bool pBeginning)
{
	// Don't bother doing aything if the Tween has not even ever started
	if(bHasPreparedTween)
	{
		// Start again at the from!
		mCurrentValue = pBeginning ? mFrom : mTo;

		switch(mTargetType)
		{
			case ETweenTargetType::UMG:
			{
				UWidget *widget = Cast<UWidget>(mTargetObject);
				UpdateForWidget(widget);
				break;
			}
			case ETweenTargetType::Custom:
			{
				break;
			}
		}
	}
}

void UTweenVector2D::OnTweenPausedInternal()
{
    Super::OnTweenPausedInternal();
}

void UTweenVector2D::OnTweenResumedInternal()
{
    Super::OnTweenResumedInternal();
}

void UTweenVector2D::OnTweenDeletedInternal()
{
    Super::OnTweenDeletedInternal();
}

void UTweenVector2D::Restart(bool pRestartFromEnd)
{
    Super::Restart(pRestartFromEnd);

    if(pRestartFromEnd)
    {
        Swap(mFrom, mTo);
    }

    mElapsedTime = 0.0f;
}

void UTweenVector2D::Invert(bool pShouldInvertElapsedTime)
{
    Super::Invert(pShouldInvertElapsedTime);

    Swap(mFrom, mTo);
}

void UTweenVector2D::BroadcastOnTweenStart()
{
    Super::BroadcastOnTweenStart();

    if(mTweenLatentProxy != nullptr)
    {
        mTweenLatentProxy->OnTweenStart.Broadcast(this);
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

void UTweenVector2D::BroadcastOnTweenUpdate()
{
    Super::BroadcastOnTweenUpdate();

    if(mTweenLatentProxy != nullptr)
    {
        mTweenLatentProxy->OnTweenUpdate.Broadcast(this);
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

void UTweenVector2D::BroadcastOnTweenEnd()
{
    Super::BroadcastOnTweenEnd();

    if(mTweenLatentProxy != nullptr)
    {
        mTweenLatentProxy->OnTweenEnd.Broadcast(this);
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
