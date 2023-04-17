// Copyright 2018 Francesco Desogus. All Rights Reserved.

#include "Tweens/TweenVector.h"
#include "TweenFactory/Latent//TweenVectorLatentFactory.h"
#include "GameFramework/Actor.h"
#include "Utils/Utility.h"
#include "Components/PrimitiveComponent.h"
#include "Utils/EaseEquations.h"
#include "Curves/CurveFloat.h"

UTweenVector::UTweenVector(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    mFrom = FVector(0, 0, 0);
    mTo = FVector(0, 0, 0);
    mCurrentValue = FVector(0, 0, 0);
    mTweenLatentProxy = nullptr;
    bDeleteTweenOnHit = false;
    bDeleteTweenOnOverlap = false;
    bHasBoundedFunctions = false;
}

/*
 **************************************************************************
 * Methods used internally. Should not be called.
 **************************************************************************
 */

void UTweenVector::Init(const FVector &pFrom, const FVector &pTo, ETweenVectorType pTweenType,
                        bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap, UTweenVectorLatentFactory *pTweenLatentProxy)
{
    mFrom = pFrom;
    mTo = pTo;
    mTweenType = pTweenType;
    mTweenLatentProxy = pTweenLatentProxy;
    bDeleteTweenOnHit = pDeleteTweenOnHit;
    bDeleteTweenOnOverlap = pDeleteTweenOnOverlap;
    bHasBoundedFunctions = false;

    mCurrentValue = FVector(0, 0, 0);
    mBaseOffset = FVector(0, 0, 0);
}


// Comments on the code are in the class UTweenFloat because they're pretty similar
bool UTweenVector::Update(float pDeltaTime, float pTimeScale)
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
			EaseEquations::ComputeCustomVectorEasing(mCurveFloat, mElapsedTime, mFrom, mTo, mTweenDuration, mCurrentValue);
		}
		else
		{
			EaseEquations::ComputeVectorEasing(mEaseType, mElapsedTime, mFrom, mTo, mTweenDuration, mCurrentValue);
		}
    }

    switch(mTargetType)
    {
        case ETweenTargetType::Actor:
        {
            AActor *actor = Cast<AActor>(mTargetObject);
            UpdateForActor(actor);
            break;
        }
        case ETweenTargetType::SceneComponent:
        {
            USceneComponent *component = Cast<USceneComponent>(mTargetObject);
            UpdateForSceneComponent(component);
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


void UTweenVector::UpdateForActor(AActor *pTarget)
{
    switch(mTweenType)
    {
        case ETweenVectorType::MoveTo:
        {
            pTarget->SetActorLocation(mCurrentValue, bHasBoundedFunctions);
            break;
        }
        case ETweenVectorType::MoveBy:
        {
             pTarget->SetActorRelativeLocation(mCurrentValue + mBaseOffset, bHasBoundedFunctions);
            break;
        }
        case ETweenVectorType::ScaleTo:
        {
            pTarget->SetActorScale3D(mCurrentValue);
            break;
        }
        case ETweenVectorType::ScaleBy:
        {
            pTarget->SetActorRelativeScale3D(mCurrentValue + mBaseOffset);
            break;
        }
        // Does nothing
        case ETweenVectorType::Custom:
        {
            break;
        }
    }


}


void UTweenVector::UpdateForSceneComponent(USceneComponent *pTarget)
{
    switch(mTweenType)
    {
        case ETweenVectorType::MoveTo:
        {
            pTarget->SetWorldLocation(mCurrentValue);
            break;
        }
        case ETweenVectorType::MoveBy:
        {
            pTarget->SetRelativeLocation(mCurrentValue + mBaseOffset/*, true*/);
            break;
        }
        case ETweenVectorType::ScaleTo:
        {
            pTarget->SetWorldScale3D(mCurrentValue);
            break;
        }
        case ETweenVectorType::ScaleBy:
        {
            pTarget->SetRelativeScale3D(mCurrentValue + mBaseOffset);
            break;
        }
        // Does nothing
        case ETweenVectorType::Custom:
        {
            break;
        }
    }
}

void UTweenVector::PreDelay()
{
    Super::PreDelay();

    BindDelegates();
}

void UTweenVector::PrepareTween()
{
    Super::PrepareTween();

    switch(mTweenType)
    {
        case ETweenVectorType::MoveTo:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::Actor:
                {
                    AActor *actor = Cast<AActor>(mTargetObject);
                    mFrom = actor->GetActorLocation();
                    break;
                }
                case ETweenTargetType::SceneComponent:
                {
                    USceneComponent *sceneComponent = Cast<USceneComponent>(mTargetObject);
                    mFrom = sceneComponent->GetComponentLocation();
                    break;
                }
            }

            break;
        }
        case ETweenVectorType::MoveBy:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::Actor:
                {
                    AActor *actor = Cast<AActor>(mTargetObject);
                    mFrom = FVector(0, 0, 0);
                    mBaseOffset = actor->GetRootComponent()->GetRelativeTransform().GetLocation();
                    break;
                }
                case ETweenTargetType::SceneComponent:
                {
                    USceneComponent *sceneComponent = Cast<USceneComponent>(mTargetObject);
                    mFrom = FVector(0, 0, 0);
                    mBaseOffset = sceneComponent->GetRelativeTransform().GetLocation();
                    break;
                }
            }

            break;
        }
        case ETweenVectorType::ScaleTo:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::Actor:
                {
                    AActor *actor = Cast<AActor>(mTargetObject);
                    mFrom = actor->GetActorScale3D();
                    break;
                }
                case ETweenTargetType::SceneComponent:
                {
                    USceneComponent *sceneComponent = Cast<USceneComponent>(mTargetObject);
                    mFrom = sceneComponent->GetComponentScale();
                    break;
                }
            }

            break;
        }
        case ETweenVectorType::ScaleBy:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::Actor:
                {
                    AActor *actor = Cast<AActor>(mTargetObject);
                    mFrom = FVector(0, 0, 0);
                    mBaseOffset = actor->GetActorRelativeScale3D();
                    break;
                }
                case ETweenTargetType::SceneComponent:
                {
                    USceneComponent *sceneComponent = Cast<USceneComponent>(mTargetObject);
                    mFrom = FVector(0, 0, 0);
                    mBaseOffset = sceneComponent->GetRelativeTransform().GetScale3D();
                    break;
                }
            }

            break;
        }
        // Does nothing
        case ETweenVectorType::Custom:
        {
            break;
        }
    }
}

void UTweenVector::PositionAt(bool pBeginning)
{
	// Don't bother doing aything if the Tween has not even ever started
	if(bHasPreparedTween)
	{
		// Start again at the from!
		mCurrentValue = pBeginning ? mFrom : mTo;

		switch(mTargetType)
		{
			case ETweenTargetType::Actor:
			{
				AActor *actor = Cast<AActor>(mTargetObject);
				UpdateForActor(actor);
				break;
			}
			case ETweenTargetType::SceneComponent:
			{
				USceneComponent *component = Cast<USceneComponent>(mTargetObject);
				UpdateForSceneComponent(component);
				break;
			}
		}
	}
}

void UTweenVector::BindDelegates()
{
    if(!bHasBoundedFunctions)
    {
        bool foundCorrectType = false;

        switch(mTweenType)
        {
            case ETweenVectorType::MoveTo:
            case ETweenVectorType::MoveBy:
            case ETweenVectorType::ScaleTo:
            case ETweenVectorType::ScaleBy:
            case ETweenVectorType::Custom:
            {
                if(AActor *actor = Cast<AActor>(mTargetObject))
                {
                    if(bDeleteTweenOnOverlap || OnTweenActorBeginOverlap.IsBound() || OnTweenActorBeginOverlapDelegate.IsBound())
                    {
                        foundCorrectType = true;
                        actor->OnActorBeginOverlap.AddDynamic(this, &UTweenVector::OnActorBeginOverlap);
                    }

                    if(bDeleteTweenOnHit || OnTweenActorHit.IsBound() || OnTweenActorHitDelegate.IsBound())
                    {
                        foundCorrectType = true;
                        actor->OnActorHit.AddDynamic(this, &UTweenVector::OnActorHit);
                    }
                }
                else if(UPrimitiveComponent *component = Cast<UPrimitiveComponent>(mTargetObject))
                {
                    if(bDeleteTweenOnOverlap || OnTweenPrimitiveComponentBeginOverlap.IsBound() || OnTweenPrimitiveComponentBeginOverlapDelegate.IsBound())
                    {
                        foundCorrectType = true;
                        component->OnComponentBeginOverlap.AddDynamic(this, &UTweenVector::OnPrimitiveComponentBeginOverlap);
                    }

                    if(bDeleteTweenOnHit || OnTweenPrimitiveComponentHit.IsBound() || OnTweenPrimitiveComponentHitDelegate.IsBound())
                    {
                        foundCorrectType = true;
                        component->OnComponentHit.AddDynamic(this, &UTweenVector::OnPrimitiveComponentHit);
                    }
                }

                break;
            }
        }

        bHasBoundedFunctions = foundCorrectType;

        if(bDeleteTweenOnHit && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenVector::BindDelegates -> DeleteTweenOnHit was selected, but the TweenTarget"
                                                "with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the selection."), *mTargetObject.Get()->GetName());
        }

        if(bDeleteTweenOnOverlap && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenVector::BindDelegates -> DeleteTweenOnOverlap was selected, but the TweenTarget"
                                                "with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the selection."), *mTargetObject.Get()->GetName());
        }

        if((OnTweenActorBeginOverlap.IsBound() || OnTweenActorBeginOverlapDelegate.IsBound()) && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenVector::BindDelegates -> OnTweenActorBeginOverlap was bound, "
                                                "but the TweenTarget with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the binding."), *mTargetObject.Get()->GetName());
        }

        if((OnTweenActorHit.IsBound() || OnTweenActorHit.IsBound()) && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenVector::BindDelegates -> OnTweenActorHit was bound, but the TweenTarget"
                                                "with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the binding."), *mTargetObject.Get()->GetName());
        }

        if((OnTweenPrimitiveComponentBeginOverlap.IsBound() || OnTweenPrimitiveComponentBeginOverlapDelegate.IsBound()) && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenVector::BindDelegates -> OnTweenPrimitiveComponentBeginOverlap was bound, "
                                                "but the TweenTarget with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the binding."), *mTargetObject.Get()->GetName());
        }

        if((OnTweenPrimitiveComponentHit.IsBound() || OnTweenPrimitiveComponentHitDelegate.IsBound()) && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenVector::BindDelegates -> OnTweenPrimitiveComponentHit was bound, but the TweenTarget"
                                                "with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the binding."), *mTargetObject.Get()->GetName());
        }
    }
}

void UTweenVector::UnbindDelegates()
{
    if(bHasBoundedFunctions)
    {
        switch(mTweenType)
        {
            case ETweenVectorType::MoveTo:
            case ETweenVectorType::MoveBy:
            case ETweenVectorType::ScaleTo:
            case ETweenVectorType::ScaleBy:
            case ETweenVectorType::Custom:
            {
                if(AActor *actor = Cast<AActor>(mTargetObject))
                {
                    if(bDeleteTweenOnOverlap || OnTweenActorBeginOverlap.IsBound() || OnTweenActorBeginOverlapDelegate.IsBound())
                    {
                        actor->OnActorBeginOverlap.RemoveAll(this);
                    }

                    if(bDeleteTweenOnHit || OnTweenActorHit.IsBound() || OnTweenActorHitDelegate.IsBound())
                    {
                        actor->OnActorHit.RemoveAll(this);
                    }
                }
                else if(UPrimitiveComponent *component = Cast<UPrimitiveComponent>(mTargetObject))
                {
                    // I can only bind the event if the tween target is actually a UPrimitiveComponent, not just a USceneComponent
                    if(bDeleteTweenOnOverlap || OnTweenPrimitiveComponentBeginOverlap.IsBound() || OnTweenPrimitiveComponentBeginOverlapDelegate.IsBound())
                    {
                        component->OnComponentBeginOverlap.RemoveAll(this);
                    }

                    if(bDeleteTweenOnHit || OnTweenPrimitiveComponentHit.IsBound() || OnTweenPrimitiveComponentHitDelegate.IsBound())
                    {
                        component->OnComponentHit.RemoveAll(this);
                    }
                }
                break;
            }
        }

        bHasBoundedFunctions = false;
    }
}


void UTweenVector::OnActorBeginOverlap(AActor *pThisActor, AActor *pOtherActor)
{
    if(IsTweening())
    {
        if(OnTweenActorBeginOverlap.IsBound())
        {
            OnTweenActorBeginOverlap.Broadcast(this, pThisActor, pOtherActor);
        }
        else if(OnTweenActorBeginOverlapDelegate.IsBound())
        {
            OnTweenActorBeginOverlapDelegate.Execute(this, pThisActor, pOtherActor);
        }

        if(bDeleteTweenOnOverlap)
        {
            DeleteTween();
        }
    }
}

void UTweenVector::OnActorHit(AActor *pThisActor, AActor *pOtherActor, FVector pNormalImpulse, const FHitResult &pHit)
{
    if(IsTweening())
    {
        if(OnTweenActorHit.IsBound())
        {
            OnTweenActorHit.Broadcast(this, pThisActor, pOtherActor, pNormalImpulse, pHit);
        }
        else if(OnTweenActorHitDelegate.IsBound())
        {
            OnTweenActorHitDelegate.Execute(this, pThisActor, pOtherActor, pNormalImpulse, pHit);
        }

        if(bDeleteTweenOnHit)
        {
            DeleteTween();
        }
    }
}

void UTweenVector::OnPrimitiveComponentBeginOverlap(UPrimitiveComponent *pThisComponent, AActor *pOtherActor, UPrimitiveComponent *pOtherComp, int32 pOtherBodyIndex, bool bFromSweep, const FHitResult &pSweepResult)
{
    if(IsTweening())
    {
        if(OnTweenPrimitiveComponentBeginOverlap.IsBound())
        {
            OnTweenPrimitiveComponentBeginOverlap.Broadcast(this, pThisComponent, pOtherActor, pOtherComp, pSweepResult);
        }
        else if(OnTweenPrimitiveComponentBeginOverlapDelegate.IsBound())
        {
            OnTweenPrimitiveComponentBeginOverlapDelegate.Execute(this, pThisComponent, pOtherActor, pOtherComp, pSweepResult);
        }

        if(bDeleteTweenOnOverlap)
        {
            DeleteTween();
        }
    }
}

void UTweenVector::OnPrimitiveComponentHit(UPrimitiveComponent *pThisComponent, AActor *pOtherActor, UPrimitiveComponent *pOtherComp, FVector pNormalImpulse, const FHitResult &pHitResult)
{
    if(IsTweening())
    {
        if(OnTweenPrimitiveComponentHit.IsBound())
        {
            OnTweenPrimitiveComponentHit.Broadcast(this, pThisComponent, pOtherActor, pOtherComp, pNormalImpulse, pHitResult);
        }
        else if(OnTweenPrimitiveComponentHitDelegate.IsBound())
        {
            OnTweenPrimitiveComponentHitDelegate.Execute(this, pThisComponent, pOtherActor, pOtherComp, pNormalImpulse, pHitResult);
        }

        if(bDeleteTweenOnHit)
        {
            DeleteTween();
        }
    }
}


void UTweenVector::OnTweenPausedInternal()
{
    Super::OnTweenPausedInternal();
}

void UTweenVector::OnTweenResumedInternal()
{
    Super::OnTweenResumedInternal();
}

void UTweenVector::OnTweenDeletedInternal()
{
    Super::OnTweenDeletedInternal();

    // Unbinding the OnHit/OnOverlap delegates if present
    UnbindDelegates();
}

void UTweenVector::Restart(bool pRestartFromEnd)
{
    Super::Restart(pRestartFromEnd);

    if(pRestartFromEnd)
    {
        Swap(mFrom, mTo);
    }

    mElapsedTime = 0.0f;
}

void UTweenVector::Invert(bool pShouldInvertElapsedTime)
{
    Super::Invert(pShouldInvertElapsedTime);

    Swap(mFrom, mTo);
}

void UTweenVector::BroadcastOnTweenStart()
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

    // Binding the OnHit/OnOverlap delegates if present and not yet bounded
    BindDelegates();
}

void UTweenVector::BroadcastOnTweenUpdate()
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

void UTweenVector::BroadcastOnTweenEnd()
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
