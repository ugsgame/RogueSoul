// Copyright 2018 Francesco Desogus. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tweens/BaseTween.h"
#include "TweenManagerComponent.h"
#include "TweenVector.generated.h"

class UTweenVectorLatentFactory;


/**
 * The UTweenVector handles the update of all Tweens that use only FVector.
 */
UCLASS(BlueprintType)
class TWEENMAKER_API UTweenVector : public UBaseTween
{
    GENERATED_BODY()

public:
    UTweenVector(const FObjectInitializer& ObjectInitializer);

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
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Current Value Info"), Category = "Tween|Info|Vector")
    FVector GetCurrentValue()
    {
        return mCurrentValue;
    }

    /*
     **************************************************************************
     * Methods used internally. Should not be called.
     **************************************************************************
     */

    void Init(const FVector &pFrom, const FVector &pTo, ETweenVectorType pTweenType, bool pDeleteTweenOnHit, bool pDeleteTweenOnOverlap, UTweenVectorLatentFactory *pTweenLatentProxy);


    ETweenVectorType GetTweenType()
    {
        return mTweenType;
    }

    /*
     **************************************************************************
     * Blueprint delegates
     **************************************************************************
     */

    /** Delegate called when the Tween starts (if it loops then it's called at each loop begin). Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Tween Start"))
    FBP_OnTweenVectorChange OnTweenStart;

    /** Delegate called at each Tween's update. Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Tween Update"))
    FBP_OnTweenVectorChange OnTweenUpdate;

    /** Delegate called when the Tween ends (if it loops then it's called at each loop end). Use only in Blueprints. */
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Tween End"))
    FBP_OnTweenVectorChange OnTweenEnd;


    /**
     * Delegate called when the Actor tween target is hit by something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from AActor and if the Tween type is either "Move" or "Scale".
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Tween Actor Hit", ScriptName = "On Tween Actor Hit"))
    FBP_OnTweenActorHitVector OnTweenActorHit;

    /**
     * Delegate called when the Actor tween target overlaps something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from AActor and if the Tween type is either "Move" or "Scale".
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Tween Actor Begin Overlap", ScriptName = "On Tween Actor Hit"))
    FBP_OnTweenActorBeginOverlapVector OnTweenActorBeginOverlap;

    /**
     * Delegate called when the PrimitiveComponent tween target is hit by something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from UPrimitiveComponent and if the Tween type is either "Move" or "Scale".
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Tween Primitive Component Hit", ScriptName = "On Tween Actor Hit"))
    FBP_OnTweenPrimitiveComponentHitVector OnTweenPrimitiveComponentHit;

    /**
     * Delegate called when the PrimitiveComponent tween target overlaps something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from UPrimitiveComponent and if the Tween type is either "Move" or "Scale".
     * Use only in Blueprints.
     */
    UPROPERTY(BlueprintAssignable, meta = (DisplayName = "On Tween Primitive Component Begin Overlap", ScriptName = "On Tween Actor Hit"))
    FBP_OnTweenPrimitiveComponentBeginOverlapVector OnTweenPrimitiveComponentBeginOverlap;


    /*
     **************************************************************************
     * C++ delegates
     **************************************************************************
     */

    /** Delegate called when the Tween starts (if it loops then it's called at each loop begin). Use only in C++. */
    FOnTweenVectorChange OnTweenStartDelegate;

    /** Delegate called at each Tween's update. Use only in C++. */
    FOnTweenVectorChange OnTweenUpdateDelegate;

    /** Delegate called when the Tween ends (if it loops then it's called at each loop end). Use only in C++. */
    FOnTweenVectorChange OnTweenEndDelegate;

    /**
     * Delegate called when the Actor tween target is hit by something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from AActor and if the Tween type is either "Move" or "Scale".
     * Use only in C++.
     */
    FOnTweenActorHitVector OnTweenActorHitDelegate;

    /**
     * Delegate called when the Actor tween target overlaps something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from AActor and if the Tween type is either "Move" or "Scale".
     * Use only in Blueprints.
     */
    FOnTweenActorBeginOverlapVector OnTweenActorBeginOverlapDelegate;

    /**
     * Delegate called when the PrimitiveComponent tween target is hit by something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from UPrimitiveComponent and if the Tween type is either "Move" or "Scale".
     * Use only in Blueprints.
     */
    FOnTweenPrimitiveComponentHitVector OnTweenPrimitiveComponentHitDelegate;

    /**
     * Delegate called when the PrimitiveComponent tween target overlaps something during the Tween's execution. Only valid if the TweenTarget
     * actually inherits from UPrimitiveComponent and if the Tween type is either "Move" or "Scale".
     * Use only in Blueprints.
     */
    FOnTweenPrimitiveComponentBeginOverlapVector OnTweenPrimitiveComponentBeginOverlapDelegate;


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
    ETweenVectorType mTweenType;

    /** Proxy for latent Tweens. It's only valid if it's a Latent tween, otherwise it's null */
    UTweenVectorLatentFactory *mTweenLatentProxy;

    /** Current value of the Tween */
    FVector mCurrentValue;

    /** Starting value */
    FVector mFrom;

    /** Ending value */
    FVector mTo;

    /** Offset used to for relative Tweens (i.e. MoveBy, ScaleBy) */
    FVector mBaseOffset;

    /** If true, the Tween will be deleted if the tween target hits something. Only valid of specific objects and specific tween types */
    bool bDeleteTweenOnHit;

    /** If true, the Tween will be deleted if the tween target overlaps something. Only valid of specific objects and specific tween types */
    bool bDeleteTweenOnOverlap;

    /** True if the Tween has bounded functions to hit/overlap events (either because the bDeleteTweenOnXYZ was selected or a method was bounded) */
    bool bHasBoundedFunctions;

private:

    /*
     **************************************************************************
     * Helper methods for updating individual classes
     **************************************************************************
     */

    void UpdateForActor(AActor *pTarget);
    void UpdateForSceneComponent(USceneComponent *pTarget);

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
    void OnPrimitiveComponentBeginOverlap(UPrimitiveComponent *pThisComponent, AActor *pOtherActor, UPrimitiveComponent *pOtherComp, int32 pOtherBodyIndex, bool bFromSweep, const FHitResult &pSweepResult);

    UFUNCTION()
    void OnPrimitiveComponentHit(UPrimitiveComponent *pThisComponent, AActor *pOtherActor, UPrimitiveComponent *pOtherComp, FVector pNormalImpulse, const FHitResult &pHitResult);
};
