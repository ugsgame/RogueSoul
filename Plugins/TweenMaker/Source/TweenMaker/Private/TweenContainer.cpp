// Copyright 2018 Francesco Desogus. All Rights Reserved.

#include "TweenContainer.h"
#include "TweenFactory/Standard/TweenVectorStandardFactory.h"
#include "TweenFactory/Standard/TweenVector2DStandardFactory.h"
#include "TweenFactory/Standard/TweenRotatorStandardFactory.h"
#include "TweenFactory/Standard/TweenLinearColorStandardFactory.h"
#include "TweenFactory/Standard/TweenFloatStandardFactory.h"
#include "Tweens/TweenVector.h"
#include "Tweens/TweenVector2D.h"
#include "Tweens/TweenRotator.h"
#include "Tweens/TweenLinearColor.h"
#include "Tweens/TweenFloat.h"
#include "TweenManagerActor.h"
#include "TweenManagerComponent.h"
#include "Utils/Utility.h"
#include "Tweens/BaseTween.h"

UTweenContainer::UTweenContainer(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer),
	  bIsPersistent(false),
	  bAreSequencesDone(false),
      bIsTweenContainerPaused(false),
	  bIsTweenContainerPendingDeletion(false),
      bHasStartedTweening(false),
	  bShouldTweenWhileGamePaused(false)
{

}



/*
 **************************************************************************
 * Methods used internally. Should not be called.
 **************************************************************************
 */

void UTweenContainer::Init(UTweenManagerComponent *pOwningTweenManager, int32 pNumLoops, ETweenLoopType pLoopType, float pTimeScale)
{
    OwningTweenManager = pOwningTweenManager;
    mNumLoops = pNumLoops;
    mLoopType = pLoopType;
    mTimeScale = pTimeScale;
    mTimeScaleAbsolute = FMath::Abs(pTimeScale);

    mSequences.Reset();
    mLoopCounter = 0;

    bIsGoingForward = mTimeScale >= 0;
    bInvertFutureTweens =!bIsGoingForward;
    bHasStartedTweening = false;
	bShouldTweenWhileGamePaused = false;

	mCurrentIndex = bIsGoingForward ? 0 : mSequences.Num() - 1;
}


void UTweenContainer::AddSequenceTween(UBaseTween *pNewTween, int32 pSequenceId)
{
    // If I need to invert future tweens (because the current timescale is < 0 or the new tween has a negative timescale, I invert it
    if(bInvertFutureTweens || pNewTween->GetTimeScale() < 0.0f)
    {
        pNewTween->ShouldInvertTween();
    }

	if(!bShouldTweenWhileGamePaused && pNewTween->bTweenWhileGameIsPaused)
	{
		bShouldTweenWhileGamePaused = true;
	}

    // Adding the Tween at the given sequence id
    mSequences.Insert(FParallelTween(pNewTween), pSequenceId);
}


// Adds it to the last sequence, if there is one; if there isn't, it will create a new sequence
void UTweenContainer::AddParallelTween(UBaseTween *pNewTween, int32 pSequenceId)
{
    // Only adding the Tween if the sequence index is valid
    if(pSequenceId >= 0)
    {
        // If I need to invert future tweens (because the current timescale is < 0 or the new tween has a negative timescale, I invert it
        if(bInvertFutureTweens || pNewTween->GetTimeScale() < 0.0f)
        {
            pNewTween->ShouldInvertTween();
        }

		if(!bShouldTweenWhileGamePaused && pNewTween->bTweenWhileGameIsPaused)
		{
			bShouldTweenWhileGamePaused = true;
		}

        // Adding the parallel Tween
        mSequences[pSequenceId].ParallelTweens.Add(pNewTween);
    }
    else
    {
        UE_LOG(LogTweenMaker, Warning, TEXT("UTweenContainer::AddParallelTween -> Trying to \"join\" a Tween in a TweenContainer that is empty. The Tween will be ignored. "
                                            "Call a \"Create\" function instead."));
    }
}


bool UTweenContainer::UpdateTweens(float pDeltaTime)
{
    bHasStartedTweening = true;

    bool areSequencesDone = false;

	// If the TweenContainer was marked as persistent and it has endend, avoid updating but pretend like it's 
	// not yet finished, so that the "outer system" won't delete the container!
	if(!bAreSequencesDone)
	{
		// Saving the current number of sequences. It might change during the update, if a user is using some callbacks that add new Tweens
		mSequencesNumAtUpdate = mSequences.Num();

		// Getting the current FParallelTween object we need to process
		FParallelTween &parallelTween = mSequences[mCurrentIndex];

		// Updating all parallel Tweens and checking if they're done
		bool areTweensDone = parallelTween.UpdateParallelTweens(pDeltaTime, mTimeScaleAbsolute);



		// If the tweens are done, I need to check if other tweens were added (because of a OnTweenEnd callback).
		// In that case, this tween object needs to be kept alive in order to handle them.
		if(areTweensDone)
		{
			bool wereThereNewAdditions = parallelTween.CheckForNewAdditions() || mSequencesNumAtUpdate < mSequences.Num();
			areTweensDone = areTweensDone && !wereThereNewAdditions;
		}

		// If the parallel Tweens are done, I need to go to the next parallel sequence
		if(areTweensDone)
		{
			// Depending if we're going forward or backward, we update the index
			mCurrentIndex = mCurrentIndex + (bIsGoingForward ? 1 : -1);

			// Have I reached the end? If so, all sequences are done
			if((bIsGoingForward && mCurrentIndex == mSequences.Num()) || (!bIsGoingForward && mCurrentIndex < 0))
			{
				// Incrementing the loop counter...
				mLoopCounter++;

				// If we didn't reach the loop count or it's infinite, let's restart
				if(mLoopCounter < mNumLoops || mNumLoops <= 0)
				{
					bool shouldRestartFromEnd = mLoopType == ETweenLoopType::Yoyo;

					// If it's "yoyo" let's switch direction
					if(shouldRestartFromEnd)
					{
						bIsGoingForward = !bIsGoingForward;
						mCurrentIndex = mCurrentIndex + (bIsGoingForward ? 1 : -1);
					}
					// If there is no "yoyo" we're definetily going forward
					else
					{
						mCurrentIndex = 0;
						bIsGoingForward = true;
					}

					// Actually restarting ALL Tweens
					for(int i = 0; i < mSequences.Num(); ++i)
					{
						FParallelTween &parallelTweenToRestart = mSequences[i];
						parallelTweenToRestart.Restart(shouldRestartFromEnd, true);
					}

				}
				// If it's not looping or we've reached the num of requested loops, let's end it here, unless it's set as persistent
				else if(bIsPersistent)
				{
					// Save the fact that it has ended...
					bAreSequencesDone = true;

					// But pretend like it's not, so the "outer system" won't delete us!
					areSequencesDone = false;
				}
				else
				{
					bAreSequencesDone = true;
					areSequencesDone = true;
				}

				// In any case, whether it's looping or not, call the container's end delegate(s), if bound
				if(OnTweenContainerEnd.IsBound())
				{
					OnTweenContainerEnd.Broadcast(this);
				}

				OnTweenContainerEndDelegate.ExecuteIfBound(this);
			}
		}
	}

    return areSequencesDone;
}


void UTweenContainer::RemoveTweensReferences(TMultiMap<TPair<TWeakObjectPtr<UObject>, ETweenGenericType>, TWeakObjectPtr<UBaseTween>> &pTweensReferenceMap)
{
    // We need to look for the Tweens owned by this TweenContainer in the Map in order to remove the references
    for(int i = 0; i < mSequences.Num(); ++i)
    {
        FParallelTween &parallelTween = mSequences[i];

        for(int j = 0; j < parallelTween.ParallelTweens.Num(); ++j)
        {
            UBaseTween *tween = parallelTween.ParallelTweens[j];

            const TPair<TWeakObjectPtr<UObject>, ETweenGenericType> *keyPair = pTweensReferenceMap.FindKey(tween);

            // The key is 100% there, but just to be safe
            if(keyPair != nullptr)
            {
                pTweensReferenceMap.RemoveSingle(*keyPair, tween);
            }
        }
    }
}

bool UTweenContainer::IsPendingDeletion()
{
    return bIsTweenContainerPendingDeletion;
}

bool UTweenContainer::IsEmpty()
{
    return mSequences.Num() == 0;
}


void UTweenContainer::InvertTweens()
{
    for(int i = 0; i < mSequences.Num(); ++i)
    {
        FParallelTween &parallelTween = mSequences[i];
        parallelTween.InvertTweens();
    }
}


int32 UTweenContainer::GetSequencesNum()
{
    return mSequences.Num();
}

void UTweenContainer::BeginDestroy()
{
    Super::BeginDestroy();

    // When the TweenContainer is being destroyed I also destroy ALL Tweens
    for(int i = 0; i < mSequences.Num(); ++i)
    {
        FParallelTween &parallelTween = mSequences[i];

        for(int j = 0; j < parallelTween.ParallelTweens.Num(); ++j)
        {
            UBaseTween *tween = parallelTween.ParallelTweens[j];
            Utility::DestroyUObject(tween);
        }
    }
}


/*
 **************************************************************************
 * "Utils" methods
 **************************************************************************
 */


void UTweenContainer::SetTimeScale(float pNewTimeScale)
{
    // If one of them is negative, the result will be negative, meaning that there
    // is a sign difference and the Tweens should be inverted
    if(pNewTimeScale * mTimeScale < 0)
    {
        bIsGoingForward = !bIsGoingForward;
        bInvertFutureTweens = !bIsGoingForward;
        InvertTweens();
    }

    mTimeScale = pNewTimeScale;
    mTimeScaleAbsolute = FMath::Abs(mTimeScale);
}

void UTweenContainer::SetLoop(int pNumLoops, ETweenLoopType pLoopType)
{
    mNumLoops = pNumLoops;
    mLoopType = pLoopType;
}

void UTweenContainer::SetPersistent(bool pPersistent)
{
	bIsPersistent = pPersistent;
}

void UTweenContainer::PauseTweenContainer()
{
    bIsTweenContainerPaused = true;
}

void UTweenContainer::TogglePauseTweenContainer()
{
    bIsTweenContainerPaused = !bIsTweenContainerPaused;
}


void UTweenContainer::ResumeTweenContainer()
{
    bIsTweenContainerPaused = false;
}

void UTweenContainer::RestartTweenContainer()
{
	bIsTweenContainerPaused = false;
	bAreSequencesDone = false;

	mLoopCounter = 0;

	// Depending on the container "direction flow", reset the position of each Tween to their starting one.
	// Example of why is needed: imagine a tween sequence in which we rotate X, then move it from A to B, then scale it.
	// If I restart the sequence when I'm at the end, it would restart rotating in B instead of A, if I wouldn't position back
	// the tweens to their starting position
	if(bIsGoingForward)
	{
		for(int i = mSequences.Num() - 1; i >= 0; --i)
		{
			mSequences[i].PositionTweensAt(true);
			mSequences[i].RestartSequence();
		}
	}
	else
	{
		for(int i = 0; i < mSequences.Num(); ++i)
		{
			mSequences[i].PositionTweensAt(false);
			mSequences[i].RestartSequence();
		}
	}

	bool wasGoingBackward = !bIsGoingForward;

	bIsGoingForward = mTimeScale >= 0;
	bInvertFutureTweens = !bIsGoingForward;
	mCurrentIndex = bIsGoingForward ? 0 : mSequences.Num() - 1;

	// If it was going backward but originally it was forward, restart the tween in the opposite sense!
	// This could probably be improved to redure the for loops in the future, but this way I can reuse previous functions.
	// It's a tricky operation, better to be done right, even if a bit slow
	if(wasGoingBackward && bIsGoingForward)
	{
		for(int i = 0; i < mSequences.Num(); ++i)
		{
			FParallelTween &parallelTweenToRestart = mSequences[i];
			parallelTweenToRestart.Restart(true, true);
		}
	}
}

void UTweenContainer::DeleteTweenContainer()
{
    bIsTweenContainerPendingDeletion = true;
}

void UTweenContainer::DeleteTweensInContainerByObject(UObject *pTweenTarget, ETweenGenericType pTweensType)
{
    // Marking the tweens for deletion. They will be actually removed from the map when the tween object is done (doesn't really
    // matter if they're not removed from the map instantly, since it's just used to quickly find tweens to delete by large groups)
    for(int i = 0; i < mSequences.Num(); ++i)
    {
        FParallelTween &parallelTween = mSequences[i];

        for(int j = 0; j < parallelTween.ParallelTweens.Num(); ++j)
        {
            UBaseTween *tween = parallelTween.ParallelTweens[j];

            // We only care about Tweens that have as target the given UObject
            if(tween->GetTweenTarget() == pTweenTarget)
            {
                switch(pTweensType)
                {
                    // If the user chose to delete "all" Tweens, we don't need to check the Tween type
                    case ETweenGenericType::Any:
                    {
                        tween->DeleteTween();
                        break;
                    }
                    default:
                    {
                        if(tween->GetTweenGenericType() == pTweensType)
                        {
                            tween->DeleteTween();
                        }
                        break;
                    }
                }
            }
        }
    }
}

bool UTweenContainer::IsObjectTweeningInContainer(UObject *pTweenTarget, ETweenGenericType pTweensType, UBaseTween *&pTweenFound)
{
    bool found = false;
    pTweenFound = nullptr;

    // Only proceding if there is a sequence being executed right now
    if(!IsPaused() && mCurrentIndex >= 0 && mCurrentIndex < mSequences.Num())
    {
        FParallelTween &parallelTweens = mSequences[mCurrentIndex];

        // Looking among all parallel tweens currently being executed
        for(int i = 0; i < parallelTweens.ParallelTweens.Num() && !found; ++i)
        {
            UBaseTween *tween = parallelTweens.ParallelTweens[i];

            // If the TweenTarget is the same, we have a candidate
            if(tween != nullptr && tween->GetTweenTarget() == pTweenTarget)
            {
                // If the type is correct and the Tween is actually tweening, we found it
                if((pTweensType == ETweenGenericType::Any || tween->GetTweenGenericType() == pTweensType) && tween->IsTweening())
                {
                    found = true;
                    pTweenFound = tween;
                }
            }
        }
    }

    return found;
}

bool UTweenContainer::HasEnded()
{
	return bAreSequencesDone;
}

bool UTweenContainer::IsPaused()
{
    return bIsTweenContainerPaused;
}


/*
 **************************************************************************
 * "Append Tween" methods
 **************************************************************************
 */

UTweenVector* UTweenContainer::AppendTweenMoveActorTo(AActor *pTweenTarget, const FVector &pTo,
                                                      float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                      int32 pNumLoops,
                                                      ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenMoveActorTo(this, pTweenTarget, pTo, pDuration, pEaseType,
                                                                  pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector* UTweenContainer::AppendTweenMoveActorBy(AActor *pTweenTarget, const FVector &pTo,
                                                      float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                      int32 pNumLoops,
                                                      ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenMoveActorBy(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector* UTweenContainer::AppendTweenScaleActorTo(AActor *pTweenTarget, const FVector &pTo,
                                                       float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                       int32 pNumLoops,
                                                       ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenScaleActorTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector* UTweenContainer::AppendTweenScaleActorBy(AActor *pTweenTarget, const FVector &pBy,
                                                       float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                       int32 pNumLoops,
                                                       ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenScaleActorBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector* UTweenContainer::AppendTweenMoveSceneComponentTo(USceneComponent *pTweenTarget, const FVector &pTo,
                                                               float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                               int32 pNumLoops,
                                                               ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenMoveSceneComponentTo(this, pTweenTarget, pTo, pDuration, pEaseType,
                                                                           pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector* UTweenContainer::AppendTweenMoveSceneComponentBy(USceneComponent *pTweenTarget, const FVector &pTo,
                                                               float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                               int32 pNumLoops,
                                                               ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenMoveSceneComponentBy(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector* UTweenContainer::AppendTweenScaleSceneComponentTo(USceneComponent *pTweenTarget, const FVector &pTo,
                                                                float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                int32 pNumLoops,
                                                                ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenScaleSceneComponentTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector* UTweenContainer::AppendTweenScaleSceneComponentBy(USceneComponent *pTweenTarget, const FVector &pBy,
                                                                float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                int32 pNumLoops,
                                                                ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenScaleSceneComponentBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector* UTweenContainer::AppendTweenCustomVector(UObject *pTweenTarget, const FVector &pFrom, const FVector &pTo,
                                                       float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                       ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_AppendTweenCustomVector(this, pTweenTarget, pFrom, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}



UTweenRotator* UTweenContainer::AppendTweenRotateActorTo(AActor *pTweenTarget, const FRotator &pTo,
                                                         float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                         int32 pNumLoops,
                                                         ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenRotatorStandardFactory::BP_AppendTweenRotateActorTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenRotator* UTweenContainer::AppendTweenRotateActorBy(AActor *pTweenTarget, const FRotator &pBy,
                                                         float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                         int32 pNumLoops,
                                                         ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenRotatorStandardFactory::BP_AppendTweenRotateActorBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenRotator* UTweenContainer::AppendTweenRotateSceneComponentTo(USceneComponent *pTweenTarget, const FRotator &pTo,
                                                                  float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                  int32 pNumLoops,
                                                                  ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenRotatorStandardFactory::BP_AppendTweenRotateSceneComponentTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenRotator* UTweenContainer::AppendTweenRotateSceneComponentBy(USceneComponent *pTweenTarget, const FRotator &pBy,
                                                                  float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                  int32 pNumLoops,
                                                                  ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenRotatorStandardFactory::BP_AppendTweenRotateSceneComponentBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenLinearColor* UTweenContainer::AppendTweenMaterialVectorFromTo(UMaterialInstanceDynamic *pTweenTarget, const FName &pParameterName, const FLinearColor &pFrom, const FLinearColor &pTo,
                                                                    float pDuration, ETweenEaseType pEaseType,
                                                                    int32 pNumLoops, ETweenLoopType pLoopType,
                                                                    float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenLinearColorStandardFactory::BP_AppendTweenMaterialVectorFromTo(this, pTweenTarget, pParameterName, pFrom, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenLinearColor* UTweenContainer::AppendTweenMaterialVectorTo(UMaterialInstanceDynamic *pTweenTarget, const FName &pParameterName, const FLinearColor &pTo,
                                                                float pDuration, ETweenEaseType pEaseType,
                                                                int32 pNumLoops, ETweenLoopType pLoopType,
                                                                float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenLinearColorStandardFactory::BP_AppendTweenMaterialVectorTo(this, pTweenTarget, pParameterName, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::AppendTweenMaterialFloatFromTo(UMaterialInstanceDynamic *pTweenTarget, const FName &pParameterName, float pFrom, float pTo,
                                                             float pDuration, ETweenEaseType pEaseType,
                                                             int32 pNumLoops, ETweenLoopType pLoopType,
                                                             float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenMaterialFloatFromTo(this, pTweenTarget, pParameterName, pFrom, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenFloat* UTweenContainer::AppendTweenMaterialFloatTo(UMaterialInstanceDynamic *pTweenTarget, const FName &pParameterName, float pTo,
                                                         float pDuration, ETweenEaseType pEaseType,
                                                         int32 pNumLoops, ETweenLoopType pLoopType,
                                                         float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenMaterialFloatTo(this, pTweenTarget, pParameterName, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenFloat* UTweenContainer::AppendTweenRotateActorAroundPoint(AActor *pTweenTarget, FVector pPivotPoint, float pStartingAngle, float pEndingAngle, float pRadius,
                                                                FVector pAxis,
                                                                float pDuration, ETweenEaseType pEaseType,
                                                                bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                int32 pNumLoops, ETweenLoopType pLoopType,
                                                                float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenRotateActorAroundPoint(this, pTweenTarget, pPivotPoint, pStartingAngle, pEndingAngle, pRadius, pAxis, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::AppendTweenRotateActorAroundPointByOffset(AActor *pTweenTarget, FVector pPivotPoint, float pOffsetAngle,
                                                                        ETweenReferenceAxis pReferenceAxis,
                                                                        float pDuration, ETweenEaseType pEaseType,
                                                                        bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                        int32 pNumLoops, ETweenLoopType pLoopType,
                                                                        float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenRotateActorAroundPointByOffset(this, pTweenTarget, pPivotPoint, pOffsetAngle, pReferenceAxis, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::AppendTweenActorFollowSpline(AActor *pTweenTarget, USplineComponent *pSpline,
                                                           float pDuration, bool pApplyRotation, bool pApplyScale,
                                                           bool pUseConstantSpeed,  ETweenEaseType pEaseType,
                                                           bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                           int32 pNumLoops, ETweenLoopType pLoopType,
                                                           float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenActorFollowSpline(this, pTweenTarget, pSpline, pDuration, pApplyRotation, pApplyScale, pUseConstantSpeed, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenFloat* UTweenContainer::AppendTweenRotateSceneComponentAroundPoint(USceneComponent *pTweenTarget, FVector pPivotPoint, float pStartingAngle, float pEndingAngle, float pRadius,
                                                                         FVector pAxis,
                                                                         float pDuration, ETweenEaseType pEaseType,
                                                                         bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                         int32 pNumLoops, ETweenLoopType pLoopType,
                                                                         float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenRotateSceneComponentAroundPoint(this, pTweenTarget, pPivotPoint, pStartingAngle, pEndingAngle, pRadius, pAxis, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenFloat* UTweenContainer::AppendTweenRotateSceneComponentAroundPointByOffset(USceneComponent *pTweenTarget, FVector pPivotPoint, float pOffsetAngle,
                                                                                 ETweenReferenceAxis pReferenceAxis,
                                                                                 float pDuration, ETweenEaseType pEaseType,
                                                                                 bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                                 int32 pNumLoops, ETweenLoopType pLoopType,
                                                                                 float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenRotateSceneComponentAroundPointByOffset(this, pTweenTarget, pPivotPoint, pOffsetAngle, pReferenceAxis, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::AppendTweenSceneComponentFollowSpline(USceneComponent *pTweenTarget, USplineComponent *pSpline,
                                                                    float pDuration, bool pApplyRotation, bool pApplyScale,
                                                                    bool pUseConstantSpeed, ETweenEaseType pEaseType,
                                                                    bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                    int32 pNumLoops, ETweenLoopType pLoopType,
                                                                    float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenSceneComponentFollowSpline(this, pTweenTarget, pSpline, pDuration, pApplyRotation, pApplyScale, pUseConstantSpeed, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}



UTweenFloat* UTweenContainer::AppendTweenWidgetAngleTo(UWidget *pTweenTarget, float pTo,
                                                       float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                       ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenWidgetAngleTo(this, pTweenTarget, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenFloat* UTweenContainer::AppendTweenWidgetOpacityTo(UWidget *pTweenTarget, float pTo,
                                                         float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                         ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenWidgetOpacityTo(this, pTweenTarget, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::AppendTweenCustomFloat(UObject *pTweenTarget, float pFrom, float pTo,
                                                     float pDuration, ETweenEaseType pEaseType,
                                                     int32 pNumLoops, ETweenLoopType pLoopType,
                                                     float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_AppendTweenCustomFloat(this, pTweenTarget, pFrom, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector2D* UTweenContainer::AppendTweenMoveWidgetTo(UWidget *pTweenTarget, const FVector2D &pTo,
                                                         float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                         ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_AppendTweenMoveWidgetTo(this, pTweenTarget, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector2D* UTweenContainer::AppendTweenMoveWidgetBy(UWidget *pTweenTarget, const FVector2D &pTo,
                                                         float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                         ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_AppendTweenMoveWidgetBy(this, pTweenTarget, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector2D* UTweenContainer::AppendTweenScaleWidgetTo(UWidget *pTweenTarget, const FVector2D &pTo,
                                                          float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                          ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_AppendTweenScaleWidgetTo(this, pTweenTarget, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector2D* UTweenContainer::AppendTweenScaleWidgetBy(UWidget *pTweenTarget, const FVector2D &pBy,
                                                          float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                          ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_AppendTweenScaleWidgetBy(this, pTweenTarget, pBy, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector2D* UTweenContainer::AppendTweenShearWidgetTo(UWidget *pTweenTarget, const FVector2D &pTo,
                                                          float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                          ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_AppendTweenShearWidgetTo(this, pTweenTarget, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector2D* UTweenContainer::AppendTweenCustomVector2D(UObject *pTweenTarget, const FVector2D &pFrom, const FVector2D &pTo,
                                                           float pDuration, ETweenEaseType pEaseType, int32 pNumLoops,
                                                           ETweenLoopType pLoopType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_AppendTweenCustomVector2D(this, pTweenTarget, pFrom, pTo, pDuration, pEaseType, pNumLoops, pLoopType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


/*
 **************************************************************************
 * "Join Tween" methods
 **************************************************************************
 */

UTweenVector* UTweenContainer::JoinTweenMoveActorTo(AActor *pTweenTarget, const FVector &pTo,
                                                    float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                    float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenMoveActorTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector* UTweenContainer::JoinTweenMoveActorBy(AActor *pTweenTarget, const FVector &pBy,
                                                    float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                    float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenMoveActorBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector* UTweenContainer::JoinTweenScaleActorTo(AActor *pTweenTarget, const FVector &pTo,
                                                     float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                     float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenScaleActorTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector* UTweenContainer::JoinTweenScaleActorBy(AActor *pTweenTarget, const FVector &pBy,
                                                     float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                     float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenScaleActorBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}



UTweenVector* UTweenContainer::JoinTweenMoveSceneComponentTo(USceneComponent *pTweenTarget, const FVector &pTo,
                                                             float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                             float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenMoveSceneComponentTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector* UTweenContainer::JoinTweenMoveSceneComponentBy(USceneComponent *pTweenTarget, const FVector &pBy,
                                                             float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                             float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenMoveSceneComponentBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}



UTweenVector* UTweenContainer::JoinTweenScaleSceneComponentTo(USceneComponent *pTweenTarget, const FVector &pTo,
                                                              float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                              float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenScaleSceneComponentTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector* UTweenContainer::JoinTweenScaleSceneComponentBy(USceneComponent *pTweenTarget, const FVector &pBy,
                                                              float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                              float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenScaleSceneComponentBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}



UTweenVector* UTweenContainer::JoinTweenCustomVector(UObject *pTweenTarget, const FVector &pFrom, const FVector &pTo,
                                                     float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVectorStandardFactory::BP_JoinTweenCustomVector(this, pTweenTarget, pFrom, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenRotator* UTweenContainer::JoinTweenRotateActorTo(AActor *pTweenTarget, const FRotator &pTo,
                                                       float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                       float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenRotatorStandardFactory::BP_JoinTweenRotateActorTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenRotator* UTweenContainer::JoinTweenRotateActorBy(AActor *pTweenTarget, const FRotator &pBy,
                                                       float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                       float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenRotatorStandardFactory::BP_JoinTweenRotateActorBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenRotator* UTweenContainer::JoinTweenRotateSceneComponentTo(USceneComponent *pTweenTarget, const FRotator &pTo,
                                                                float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenRotatorStandardFactory::BP_JoinTweenRotateSceneComponentTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenRotator* UTweenContainer::JoinTweenRotateSceneComponentBy(USceneComponent *pTweenTarget, const FRotator &pBy,
                                                                float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenRotatorStandardFactory::BP_JoinTweenRotateSceneComponentBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}



UTweenLinearColor* UTweenContainer::JoinTweenMaterialVectorFromTo(UMaterialInstanceDynamic *pTweenTarget, const FName &pParameterName, const FLinearColor &pFrom, const FLinearColor &pTo,
                                                                  float pDuration, ETweenEaseType pEaseType,
                                                                  float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenLinearColorStandardFactory::BP_JoinTweenMaterialVectorFromTo(this, pTweenTarget, pParameterName, pFrom, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenLinearColor* UTweenContainer::JoinTweenMaterialVectorTo(UMaterialInstanceDynamic *pTweenTarget, const FName &pParameterName, const FLinearColor &pTo,
                                                              float pDuration, ETweenEaseType pEaseType,
                                                              float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenLinearColorStandardFactory::BP_JoinTweenMaterialVectorTo(this, pTweenTarget, pParameterName, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}



UTweenFloat* UTweenContainer::JoinTweenMaterialFloatFromTo(UMaterialInstanceDynamic *pTweenTarget, const FName &pParameterName, float pFrom, float pTo,
                                                           float pDuration, ETweenEaseType pEaseType,
                                                           float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenMaterialFloatFromTo(this, pTweenTarget, pParameterName, pFrom, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenMaterialFloatTo(UMaterialInstanceDynamic *pTweenTarget, const FName &pParameterName, float pTo,
                                                       float pDuration, ETweenEaseType pEaseType,
                                                       float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenMaterialFloatTo(this, pTweenTarget, pParameterName, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenRotateActorAroundPoint(AActor *pTweenTarget, FVector pPivotPoint, float pStartingAngle, float pEndingAngle, float pRadius,
                                                              FVector pAxis,
                                                              float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                              float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenRotateActorAroundPoint(this, pTweenTarget, pPivotPoint, pStartingAngle, pEndingAngle, pRadius, pAxis, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenRotateActorAroundPointByOffset(AActor *pTweenTarget, FVector pPivotPoint, float pOffsetAngle,
                                                                      ETweenReferenceAxis pReferenceAxis,
                                                                      float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                      float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenRotateActorAroundPointByOffset(this, pTweenTarget, pPivotPoint, pOffsetAngle, pReferenceAxis, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenActorFollowSpline(AActor *pTweenTarget, USplineComponent *pSpline,
                                                         float pDuration, bool pApplyRotation, bool pApplyScale,
                                                         bool pUseConstantSpeed, ETweenEaseType pEaseType,
                                                         bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                         float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenActorFollowSpline(this, pTweenTarget, pSpline, pDuration, pApplyRotation, pApplyScale, pUseConstantSpeed, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenRotateSceneComponentAroundPoint(USceneComponent *pTweenTarget, FVector pPivotPoint, float pStartingAngle, float pEndingAngle, float pRadius,
                                                                       FVector pAxis,
                                                                       float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                       float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenRotateSceneComponentAroundPoint(this, pTweenTarget, pPivotPoint, pStartingAngle, pEndingAngle, pRadius, pAxis, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenRotateSceneComponentAroundPointByOffset(USceneComponent *pTweenTarget, FVector pPivotPoint, float pOffsetAngle,
                                                                               ETweenReferenceAxis pReferenceAxis,
                                                                               float pDuration, ETweenEaseType pEaseType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                               float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenRotateSceneComponentAroundPointByOffset(this, pTweenTarget, pPivotPoint, pOffsetAngle, pReferenceAxis, pDuration, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenSceneComponentFollowSpline(USceneComponent *pTweenTarget, USplineComponent *pSpline,
                                                                  float pDuration, bool pApplyRotation, bool pApplyScale,
                                                                  bool pUseConstantSpeed, ETweenEaseType pEaseType,
                                                                  bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap,
                                                                  float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenSceneComponentFollowSpline(this, pTweenTarget, pSpline, pDuration, pApplyRotation, pApplyScale, pUseConstantSpeed, pEaseType, pDeleteTweenOnHit, pDeleteTweenOnOverlap, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenWidgetAngleTo(UWidget *pTweenTarget, float pTo,
                                                     float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenWidgetAngleTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenFloat* UTweenContainer::JoinTweenWidgetOpacityTo(UWidget *pTweenTarget, float pTo,
                                                       float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenWidgetOpacityTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenFloat* UTweenContainer::JoinTweenCustomFloat(UObject *pTweenTarget, float pFrom, float pTo,
                                                   float pDuration, ETweenEaseType pEaseType,
                                                   float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenFloatStandardFactory::BP_JoinTweenCustomFloat(this, pTweenTarget, pFrom, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}



UTweenVector2D* UTweenContainer::JoinTweenMoveWidgetTo(UWidget *pTweenTarget, const FVector2D &pTo,
                                                       float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_JoinTweenMoveWidgetTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector2D* UTweenContainer::JoinTweenMoveWidgetBy(UWidget *pTweenTarget, const FVector2D &pBy,
                                                       float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_JoinTweenMoveWidgetBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector2D* UTweenContainer::JoinTweenScaleWidgetTo(UWidget *pTweenTarget, const FVector2D &pTo,
                                                        float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_JoinTweenScaleWidgetTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}

UTweenVector2D* UTweenContainer::JoinTweenScaleWidgetBy(UWidget *pTweenTarget, const FVector2D &pBy,
                                                        float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_JoinTweenScaleWidgetBy(this, pTweenTarget, pBy, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector2D* UTweenContainer::JoinTweenShearWidgetTo(UWidget *pTweenTarget, const FVector2D &pTo,
                                                        float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_JoinTweenShearWidgetTo(this, pTweenTarget, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}


UTweenVector2D* UTweenContainer::JoinTweenCustomVector2D(UObject *pTweenTarget, const FVector2D &pFrom, const FVector2D &pTo,
                                                         float pDuration, ETweenEaseType pEaseType, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused)
{
    return UTweenVector2DStandardFactory::BP_JoinTweenCustomVector2D(this, pTweenTarget, pFrom, pTo, pDuration, pEaseType, pDelay, pTimeScale, pTweenWhileGameIsPaused);
}
