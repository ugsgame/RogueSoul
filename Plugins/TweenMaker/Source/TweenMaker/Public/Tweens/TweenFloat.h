// Copyright 2018 Francesco Desogus. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tweens/BaseTween.h"
#include "TweenManagerComponent.h"
#include "TweenFloat.generated.h"

class UTweenFloatLatentFactory;
class UMaterialInstanceDynamic;
class USplineComponent;
class UWidget;


/**
 * The UTweenFloat handles the update of all Tweens that use only floats.
 */
UCLASS(BlueprintType)
class TWEENMAKER_API UTweenFloat : public UBaseTween
{
    GENERATED_BODY()

public:
    UTweenFloat(const FObjectInitializer& ObjectInitializer);

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
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Current Value Info"), Category = "Tween|Info|Float")
    float GetCurrentValue()
    {
        return mCurrentValue;
    }

    /*
     **************************************************************************
     * Methods used internally. Should not be called.
     **************************************************************************
     */

    /** Init method for a generic TweenFloat */
    void Init(float pFrom, float pTo, ETweenFloatType pTweenType, const FName &pParameterName, UTweenFloatLatentFactory *pTweenLatentProxy);

    /** Init method for a "RotateAround" type of TweenFloat */
    void InitRotateAround(const FVector &pPoint, float pStartingAngle, float pEndingAngle, float pRadius,
                          const FVector &pAxis, ETweenFloatType pTweenType, bool pDeleteTweenOnHit,
                          bool pDeleteTweenOnOverlap, UTweenFloatLatentFactory *pTweenLatentProxy);

    /** Init method for a "FollowSpline" type of TweenFloat */
    void InitFollowSpline(USplineComponent *pSpline, bool pApplyRotation, bool pApplyScale, ETweenFloatType pTweenType,
                          bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap, bool pUseConstantSpeed, UTweenFloatLatentFactory *pTweenLatentProxy);


    ETweenFloatType GetTweenType()
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
    FBP_OnTweenFloatChange OnTweenStart;

    /** Delegate called at each Tween's update. Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenFloatChange OnTweenUpdate;

    /** Delegate called when the Tween ends (if it loops then it's called at each loop end). Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenFloatChange OnTweenEnd;



    /**
     * Delegate called when the Actor tween target is hit by something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from AActor and if the Tween type is either "FollowSpline" or "RotateAround".
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenActorHitFloat OnTweenActorHit;

    /**
     * Delegate called when the Actor tween target overlaps something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from AActor and if the Tween type is either "FollowSpline" or "RotateAround".
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenActorBeginOverlapFloat OnTweenActorBeginOverlap;


    /**
     * Delegate called when the PrimitiveComponent tween target is hit by something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from UPrimitiveComponent and if the Tween type is either "FollowSpline" or "RotateAround".
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenPrimitiveComponentHitFloat OnTweenPrimitiveComponentHit;

    /**
     * Delegate called when the PrimitiveComponent tween target overlaps something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from UPrimitiveComponent and if the Tween type is either "FollowSpline" or "RotateAround".
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenPrimitiveComponentBeginOverlapFloat OnTweenPrimitiveComponentBeginOverlap;

    /**
     * Delegate called whenever a "FollowSpline" Tween reaches a point of the spline itself.
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable)
    FBP_OnTweenSplinePointReached OnTweenSplinePointReached;


    /*
     **************************************************************************
     * C++ delegates
     **************************************************************************
     */

    /** Delegate called when the Tween starts (if it loops then it's called at each loop begin). Use only in C++. */
    FOnTweenFloatChange OnTweenStartDelegate;

    /** Delegate called at each Tween's update. Use only in C++. */
    FOnTweenFloatChange OnTweenUpdateDelegate;

    /** Delegate called when the Tween ends (if it loops then it's called at each loop end). Use only in C++. */
    FOnTweenFloatChange OnTweenEndDelegate;



    /**
     * Delegate called when the Actor tween target is hit by something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from AActor and if the Tween type is either "FollowSpline" or "RotateAround".
     * Use only in C++.
     */
    FOnTweenActorHitFloat OnTweenActorHitDelegate;

    /**
     * Delegate called when the Actor tween target overlaps something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from AActor and if the Tween type is either "FollowSpline" or "RotateAround".
     * Use only in C++.
     */
    FOnTweenActorBeginOverlapFloat OnTweenActorBeginOverlapDelegate;

    /**
     * Delegate called when the PrimitiveComponent tween target is hit by something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from UPrimitiveComponent and if the Tween type is either "FollowSpline" or "RotateAround".
     * Use only in C++.
     */
    FOnTweenPrimitiveComponentHitFloat OnTweenPrimitiveComponentHitDelegate;

    /**
     * Delegate called when the PrimitiveComponent tween target overlaps something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from UPrimitiveComponent and if the Tween type is either "FollowSpline" or "RotateAround".
     * Use only in C++.
     */
    FOnTweenPrimitiveComponentBeginOverlapFloat OnTweenPrimitiveComponentBeginOverlapDelegate;

    /**
     * Delegate called whenever a "FollowSpline" Tween reaches a point of the spline itself.
     * Use only in C++.
     */
    FOnTweenSplinePointReached OnTweenSplinePointReachedDelegate;


protected:
    // Begin of UBaseTween interface
    virtual bool Update(float pDeltaTime, float pTimeScale) override;
    virtual void Restart(bool pRestartFromEnd) override;
    virtual void Invert(bool pShouldInvertElapsedTime = true) override;
    virtual void PrepareTween() override;
	virtual void PositionAt(bool pBeginning) override;
    virtual void PreDelay() override;
    virtual void BroadcastOnTweenStart() override;
    virtual void BroadcastOnTweenUpdate() override;
    virtual void BroadcastOnTweenEnd() override;
    virtual void OnTweenPausedInternal() override;
    virtual void OnTweenResumedInternal() override;
    virtual void OnTweenDeletedInternal() override;
    // End of UBaseTween interface
private:

    /** Type of this Tween */
    ETweenFloatType mTweenType;

    /** Proxy for latent Tweens. It's only valid if it's a Latent tween, otherwise it's null */
    UTweenFloatLatentFactory *TweenLatentProxy;

    /** Current value of the Tween */
    float mCurrentValue;

    /** Starting value */
    float mFrom;

    /** Ending value */
    float mTo;

    /** Previous value */
    float mPreviousValue;

    /** If true, the Tween will be deleted if the tween target hits something. Only valid of specific objects and specific tween types */
    bool bDeleteTweenOnHit;

    /** If true, the Tween will be deleted if the tween target overlaps something. Only valid of specific objects and specific tween types */
    bool bDeleteTweenOnOverlap;

    /** True if the Tween has bounded functions to hit/overlap events (either because the bDeleteTweenOnXYZ was selected or a method was bounded) */
    bool bHasBoundedFunctions;

    /** Name of the material parameter. It's only valid if the Tween's type deals with materials */
    FName mParameterName;

    // Begin of variables used for "RotateAround" type of Tweens
    FVector mOffsetVector;
    FVector mPivotPoint;
    float mCurrentAngle;
    float mRadius;
    FVector mAxis;

	// Spline related fields
	int mNextPointIndex; // Keeps track of the next point of the spline (meaningful only when the relative delegate is bound)

    // Begin of variables used for "FollowSpline" type of Tweens
    USplineComponent *mSpline;
    bool bSplineApplyRotation;
    bool bSplineApplyScale;
    bool bUseConstantSpeed;

private:

    void CheckSplinePointReached(const FVector& WorldPosition);

    /*
     **************************************************************************
     * Helper methods for updating individual classes
     **************************************************************************
     */

    void UpdateForMaterial(UMaterialInstanceDynamic *pTarget);
    void UpdateForActor(AActor *pTarget);
    void UpdateForSceneComponent(USceneComponent *pTarget);
    void UpdateForWidget(UWidget *pTarget);

    /*
     **************************************************************************
     * Helper methods for binding/unbinding hit and overlap delegates
     **************************************************************************
     */


    void BindDelegates();
    void UnbindDelegates();

    UFUNCTION()
    void OnActorBeginOverlap(AActor *pThisActor, AActor *pOtherActor);

    UFUNCTION()
    void OnActorHit(AActor *pThisActor, AActor *pOtherActor, FVector pNormalImpulse, const FHitResult &pHit);

    UFUNCTION()
    void OnPrimitiveComponentBeginOverlap(UPrimitiveComponent *pThisComponent, AActor *pOtherActor, UPrimitiveComponent *pOtherComp,
                                          int32 pOtherBodyIndex, bool bFromSweep, const FHitResult &pSweepResult);

    UFUNCTION()
    void OnPrimitiveComponentHit(UPrimitiveComponent *pThisComponent, AActor *pOtherActor, UPrimitiveComponent *pOtherComp,
                                 FVector pNormalImpulse, const FHitResult &pHitResult);

};
