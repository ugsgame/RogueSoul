// Copyright 2018 Francesco Desogus. All Rights Reserved.

#include "Tweens/TweenRotator.h"
#include "TweenFactory/Latent//TweenRotatorLatentFactory.h"
#include "GameFramework/Actor.h"
#include "Utils/Utility.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/EaseEquations.h"

UTweenRotator::UTweenRotator(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    mFrom = FQuat::Identity;
    mTo = FQuat::Identity;
    mFromRotator = FRotator::ZeroRotator;
    mToRotator = FRotator::ZeroRotator;
    mCurrentValue = FQuat::Identity;
    mTweenLatentProxy = nullptr;
    bDeleteTweenOnHit = false;
    bDeleteTweenOnOverlap = false;
    bHasBoundedFunctions = false;

    mBaseOffset = FQuat::Identity;
}

/*
 **************************************************************************
 * Methods used internally. Should not be called.
 **************************************************************************
 */

void UTweenRotator::Init(const FRotator &pFrom, const FRotator &pTo, ETweenRotatorType pTweenType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap, UTweenRotatorLatentFactory *pTweenLatentProxy)
{
    mFrom = pFrom.Quaternion();
    mTo = pTo.Quaternion();
    mFromRotator = pFrom;
    mToRotator = pTo;
    mTweenType = pTweenType;
    mTweenLatentProxy = pTweenLatentProxy;
    bDeleteTweenOnHit = pDeleteTweenOnHit;
    bDeleteTweenOnOverlap = pDeleteTweenOnOverlap;
    bHasBoundedFunctions = false;

    mCurrentValue = FQuat::Identity;
    mBaseOffset = FQuat::Identity;
}

// Comments on the code are in the class UTweenFloat because they're pretty similar
bool UTweenRotator::Update(float pDeltaTime, float pTimeScale)
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
        // To rotate I first compute the float for the spherical interpolation
		float alpha = 0;

		if(mCurveFloat)
		{
			alpha = EaseEquations::ComputeCustomFloatEasing(mCurveFloat, mElapsedTime, 0, 1.0f, mTweenDuration);
		}
		else
		{
			alpha = EaseEquations::ComputeFloatEasing(mEaseType, mElapsedTime, 0, 1.0f, mTweenDuration);
		}

        if(mTweenType == ETweenRotatorType::RotateBy)
        {
            float pitch = FMath::Lerp(mFromRotator.Pitch, mToRotator.Pitch, alpha);
            float yaw = FMath::Lerp(mFromRotator.Yaw, mToRotator.Yaw, alpha);
            float roll = FMath::Lerp(mFromRotator.Roll, mToRotator.Roll, alpha);

            FRotator rot = FRotator(pitch, yaw, roll);
            mCurrentValue = rot.Quaternion();
        }
        else
        {
            mCurrentValue = FQuat::Slerp(mFrom, mTo, alpha);
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

void UTweenRotator::UpdateForActor(AActor *pTarget)
{
    switch(mTweenType)
    {
        case ETweenRotatorType::RotateTo:
        {
            pTarget->SetActorRotation(mCurrentValue.Rotator());
            break;
        }
        case ETweenRotatorType::RotateBy:
        {
            pTarget->SetActorRelativeRotation(UKismetMathLibrary::ComposeRotators(mBaseOffset.Rotator(), mCurrentValue.Rotator()));
            break;
        }
    }
}


void UTweenRotator::UpdateForSceneComponent(USceneComponent *pTarget)
{
    switch(mTweenType)
    {
        case ETweenRotatorType::RotateTo:
        {
            pTarget->SetWorldRotation(mCurrentValue.Rotator());
            break;
        }
        case ETweenRotatorType::RotateBy:
        {
            pTarget->SetRelativeRotation(UKismetMathLibrary::ComposeRotators(mBaseOffset.Rotator(), mCurrentValue.Rotator()));
            break;
        }
    }
}

void UTweenRotator::PreDelay()
{
    Super::PreDelay();

    BindDelegates();
}

void UTweenRotator::PrepareTween()
{
    Super::PrepareTween();

    switch(mTweenType)
    {
        case ETweenRotatorType::RotateTo:
        {
            switch(mTargetType)
            {
                case ETweenTargetType::Actor:
                {
                    AActor *actor = Cast<AActor>(mTargetObject);
                    mFrom = actor->GetActorRotation().Quaternion();
                    break;
                }
                case ETweenTargetType::SceneComponent:
                {
                    USceneComponent *sceneComponent = Cast<USceneComponent>(mTargetObject);
                    mFrom = sceneComponent->GetComponentRotation().Quaternion();
                    break;
                }
            }

            break;
        }
        case ETweenRotatorType::RotateBy:
        {
            mFrom = FQuat::Identity;
            mFromRotator = FRotator::ZeroRotator;

            switch(mTargetType)
            {

                case ETweenTargetType::Actor:
                {
                    AActor *actor = Cast<AActor>(mTargetObject);
                    mBaseOffset = actor->GetRootComponent()->GetRelativeTransform().GetRotation();
                    break;
                }
                case ETweenTargetType::SceneComponent:
                {
                    USceneComponent *sceneComponent = Cast<USceneComponent>(mTargetObject);
                    mBaseOffset = sceneComponent->GetRelativeTransform().GetRotation();
                    break;
                }
            }

            break;
        }
    }
}

void UTweenRotator::PositionAt(bool pBeginning)
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

void UTweenRotator::BindDelegates()
{
    // Procede only if no functions were bounded yet
    if(!bHasBoundedFunctions)
    {
        bool foundCorrectType = false;

        switch(mTweenType)
        {
        case ETweenRotatorType::RotateBy:
        case ETweenRotatorType::RotateTo:
            {
                if(AActor *actor = Cast<AActor>(mTargetObject))
                {
                    if(bDeleteTweenOnOverlap || OnTweenActorBeginOverlap.IsBound() || OnTweenActorBeginOverlapDelegate.IsBound())
                    {
                        foundCorrectType = true;
                        actor->OnActorBeginOverlap.AddDynamic(this, &UTweenRotator::OnActorBeginOverlap);
                    }

                    if(bDeleteTweenOnHit || OnTweenActorHit.IsBound() || OnTweenActorHitDelegate.IsBound())
                    {
                        foundCorrectType = true;
                        actor->OnActorHit.AddDynamic(this, &UTweenRotator::OnActorHit);
                    }
                }
                else if(UPrimitiveComponent *component = Cast<UPrimitiveComponent>(mTargetObject))
                {
                    // I can only bind the event if the tween target is actually a UPrimitiveComponent, not just a USceneComponent
                    if(bDeleteTweenOnOverlap || OnTweenPrimitiveComponentBeginOverlap.IsBound() || OnTweenPrimitiveComponentBeginOverlapDelegate.IsBound())
                    {
                        foundCorrectType = true;
                        component->OnComponentBeginOverlap.AddDynamic(this, &UTweenRotator::OnPrimitiveComponentBeginOverlap);
                    }

                    if(bDeleteTweenOnHit || OnTweenPrimitiveComponentHit.IsBound() || OnTweenPrimitiveComponentHitDelegate.IsBound())
                    {
                        foundCorrectType = true;
                        component->OnComponentHit.AddDynamic(this, &UTweenRotator::OnPrimitiveComponentHit);
                    }
                }
                break;
            }
        }

        bHasBoundedFunctions = foundCorrectType;

        if(bDeleteTweenOnOverlap && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenRotator::BindDelegates -> DeleteTweenOnOverlap was selected, but the TweenTarget"
                                                "with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the selection."), *mTargetObject.Get()->GetName());
        }

        if(bDeleteTweenOnHit && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenRotator::BindDelegates -> bDeleteTweenOnHit was selected, but the TweenTarget"
                                                "with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the selection."), *mTargetObject.Get()->GetName());
        }

        if((OnTweenActorBeginOverlap.IsBound() || OnTweenActorBeginOverlapDelegate.IsBound()) && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenRotator::BindDelegates -> OnTweenActorBeginOverlap was bound, "
                                                "but the TweenTarget with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the binding."), *mTargetObject.Get()->GetName());
        }

        if((OnTweenActorHit.IsBound() || OnTweenActorHit.IsBound()) && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenRotator::BindDelegates -> OnTweenActorHit was bound, but the TweenTarget"
                                                "with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the binding."), *mTargetObject.Get()->GetName());
        }

        if((OnTweenPrimitiveComponentBeginOverlap.IsBound() || OnTweenPrimitiveComponentBeginOverlapDelegate.IsBound()) && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenRotator::BindDelegates -> OnTweenPrimitiveComponentBeginOverlap was bound, "
                                                "but the TweenTarget with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the binding."), *mTargetObject.Get()->GetName());
        }

        if((OnTweenPrimitiveComponentHit.IsBound() || OnTweenPrimitiveComponentHitDelegate.IsBound()) && !foundCorrectType)
        {
            UE_LOG(LogTweenMaker, Warning, TEXT("UTweenRotator::BindDelegates -> OnTweenPrimitiveComponentHit was bound, but the TweenTarget"
                                                "with name %s doesn't derive from an Actor nor a PrimitiveComponent. "
                                                "Ignoring the binding."), *mTargetObject.Get()->GetName());
        }
    }
}

void UTweenRotator::UnbindDelegates()
{
    if(bHasBoundedFunctions)
    {
        switch(mTweenType)
        {
            case ETweenRotatorType::RotateBy:
            case ETweenRotatorType::RotateTo:
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


void UTweenRotator::OnActorBeginOverlap(AActor *pThisActor, AActor *pOtherActor)
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

void UTweenRotator::OnActorHit(AActor *pThisActor, AActor *pOtherActor, FVector pNormalImpulse, const FHitResult &pHit)
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

void UTweenRotator::OnPrimitiveComponentBeginOverlap(UPrimitiveComponent *pThisComponent, AActor *pOtherActor, UPrimitiveComponent *pOtherComp, int32 pOtherBodyIndex, bool bFromSweep, const FHitResult &pSweepResult)
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

void UTweenRotator::OnPrimitiveComponentHit(UPrimitiveComponent *pThisComponent, AActor *pOtherActor, UPrimitiveComponent *pOtherComp, FVector pNormalImpulse, const FHitResult &pHitResult)
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


void UTweenRotator::OnTweenPausedInternal()
{
    Super::OnTweenPausedInternal();
}

void UTweenRotator::OnTweenResumedInternal()
{
    Super::OnTweenResumedInternal();
}

void UTweenRotator::OnTweenDeletedInternal()
{
    Super::OnTweenDeletedInternal();

    UnbindDelegates();
}

void UTweenRotator::Restart(bool pRestartFromEnd)
{
    Super::Restart(pRestartFromEnd);

    if(pRestartFromEnd)
    {
        Swap(mFrom, mTo);
        Swap(mFromRotator, mToRotator);
    }

    mElapsedTime = 0.0f;
}

void UTweenRotator::Invert(bool pShouldInvertElapsedTime)
{
    Super::Invert(pShouldInvertElapsedTime);

    Swap(mFrom, mTo);
    Swap(mFromRotator, mToRotator);
}

void UTweenRotator::BroadcastOnTweenStart()
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

    // Binding the OnHit/OnOverlap delegates if present
    BindDelegates();
}

void UTweenRotator::BroadcastOnTweenUpdate()
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

void UTweenRotator::BroadcastOnTweenEnd()
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
