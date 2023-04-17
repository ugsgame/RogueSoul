// Copyright 2018 Francesco Desogus. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tweens/BaseTween.h"
#include "TweenManagerComponent.h"
#include "TweenVector2D.generated.h"

class UTweenVector2DLatentFactory;
class UWidget;


/**
 * The UTweenVector2D handles the update of all Tweens that use only FVector2D.
 */
UCLASS(BlueprintType)
class TWEENMAKER_API UTweenVector2D : public UBaseTween
{
    GENERATED_BODY()

public:
    UTweenVector2D(const FObjectInitializer& ObjectInitializer);


    /*
     **************************************************************************
     * "Tween Info" methods
     **************************************************************************
     */

    /**
      * Returns the current value of the Tween.
      *
      * @return The current value.
      */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Current Value Info"), Category = "Tween|Info|Vector2D")
    FVector2D GetCurrentValue()
    {
        return mCurrentValue;
    }

    /*
     **************************************************************************
     * Methods used internally. Should not be called.
     **************************************************************************
     */

    void Init(const FVector2D &pFrom, const FVector2D &pTo, ETweenVector2DType pTweenType, UTweenVector2DLatentFactory *pTweenLatentProxy);


    ETweenVector2DType GetTweenType()
    {
        return mTweenType;
    }


    /*
     **************************************************************************
     * Blueprint delegates
     **************************************************************************
     */


    /** Delegate called when the Tween starts (if it loops then it's called at each loop begin). Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenVector2DChange OnTweenStart;

    /** Delegate called at each Tween's update. Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenVector2DChange OnTweenUpdate;

    /** Delegate called when the Tween ends (if it loops then it's called at each loop end). Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenVector2DChange OnTweenEnd;

    /*
     **************************************************************************
     * C++ delegates
     **************************************************************************
     */


    /** Delegate called when the Tween starts (if it loops then it's called at each loop begin). Use only in C++. */
    FOnTweenVector2DChange OnTweenStartDelegate;

    /** Delegate called at each Tween's update. Use only in C++. */
    FOnTweenVector2DChange OnTweenUpdateDelegate;

    /** Delegate called when the Tween ends (if it loops then it's called at each loop end). Use only in C++. */
    FOnTweenVector2DChange OnTweenEndDelegate;


protected:
    // Begin of UBaseTween interface
    virtual bool Update(float pDeltaTime, float pTimeScale) override;
    virtual void Restart(bool pRestartFromEnd) override;
    virtual void Invert(bool pShouldInvertElapsedTime = true) override;
    virtual void PrepareTween() override;
	virtual void PositionAt(bool pBeginning) override;
	virtual void BroadcastOnTweenStart() override;
    virtual void BroadcastOnTweenUpdate() override;
    virtual void BroadcastOnTweenEnd() override;
    virtual void OnTweenPausedInternal() override;
    virtual void OnTweenResumedInternal() override;
    virtual void OnTweenDeletedInternal() override;
    // End of UBaseTween interface

private:
    /** Type of this Tween */
    ETweenVector2DType mTweenType;

    /** Proxy for latent Tweens. It's only valid if it's a Latent tween, otherwise it's null */
    UTweenVector2DLatentFactory *mTweenLatentProxy;

    /** Current value of the Tween */
    FVector2D mCurrentValue;

    /** Starting value */
    FVector2D mFrom;

    /** Ending value */
    FVector2D mTo;

    /** Offset used to for relative Tweens (i.e. MoveBy, ScaleBy) */
    FVector2D mBaseOffset;

private:

    /*
     **************************************************************************
     * Helper methods for updating individual classes
     **************************************************************************
     */

    void UpdateForWidget(UWidget *pTarget);
};
