// Copyright 2018 Francesco Desogus. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tweens/BaseTween.h"
#include "TweenManagerComponent.h"
#include "TweenLinearColor.generated.h"

class UTweenLinearColorLatentFactory;
class UMaterialInstanceDynamic;


/**
 * The UTweenLinearColor handles the update of all Tweens that use only FLinearColor.
 */
UCLASS(BlueprintType)
class TWEENMAKER_API UTweenLinearColor : public UBaseTween
{
    GENERATED_BODY()

public:
    UTweenLinearColor(const FObjectInitializer& ObjectInitializer);

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
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Current Value Info"), Category = "Tween|Info|LinearColor")
    FLinearColor GetCurrentValue()
    {
        return CurrentValue;
    }

    /*
     **************************************************************************
     * Methods used internally. Should not be called.
     **************************************************************************
     */

    void Init(const FName &pParameterName, const FLinearColor &pFrom, const FLinearColor &pTo,
              ETweenLinearColorType pTweenType, UTweenLinearColorLatentFactory *pTweenLatentProxy);

    ETweenLinearColorType GetTweenType()
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
    FBP_OnTweenLinearColorChange OnTweenStart;

    /** Delegate called at each Tween's update. Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenLinearColorChange OnTweenUpdate;

    /** Delegate called when the Tween ends (if it loops then it's called at each loop end). Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenLinearColorChange OnTweenEnd;

    /*
     **************************************************************************
     * C++ delegates
     **************************************************************************
     */

    /** Delegate called when the Tween starts (if it loops then it's called at each loop begin). Use only in C++. */
    FOnTweenLinearColorChange OnTweenStartDelegate;

    /** Delegate called at each Tween's update. Use only in C++. */
    FOnTweenLinearColorChange OnTweenUpdateDelegate;

    /** Delegate called when the Tween ends (if it loops then it's called at each loop end). Use only in C++. */
    FOnTweenLinearColorChange OnTweenEndDelegate;


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
    ETweenLinearColorType mTweenType;

    /** Proxy for latent Tweens. It's only valid if it's a Latent tween, otherwise it's null */
    UTweenLinearColorLatentFactory *TweenLatentProxy;

    /** Name of the material parameter to tween. */
    FName mParameterName;

    /** Current value of the Tween */
    FLinearColor CurrentValue;

    /** Starting value */
    FLinearColor From;

    /** Ending value */
    FLinearColor To;

    /** Helper variables */
    FVector FromVector;
    FVector ToVector;
    FVector CurrentValueVector;

private:

    /*
     **************************************************************************
     * Helper methods for updating individual classes
     **************************************************************************
     */

    void UpdateForMaterial(UMaterialInstanceDynamic *pTarget);
};
