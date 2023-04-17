// Copyright 2018 Francesco Desogus. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Utils/TweenEnums.h"
#include "Utils/Utility.h"
#include "BaseTween.generated.h"

class UMaterialInsstanceDynamic;
class UWidget;
class UCurveFloat;
class UTweenContainer;
class UTweenVector;
class UTweenRotator;
class UTweenLinearColor;
class UTweenFloat;
class UTweenVector2D;


/// @brief Delegate emitted whenever the name of the Tween changes. Used to update the internal map, for easier access
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTweenNameChanged, UBaseTween*, pTween, const FName&, pPreviousName, const FName&, pNewName);

/// @brief Delegate emitted right before the Tween is destroyed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTweenDestroyed, UBaseTween*, pTween);

/**
 * The UBaseTween class is the base class for all type of Tweens. It provides all common functionalities,
 * such as the ability to pause/resume/delete Tweens and makes it easier for derived Tweens to call callbacks.
 *
 * It also provides a convenient way to append/join new Tweens to the instanced Tween by implementing methods that
 * forward the creation call to the corresponding StandardFactory (Latent Tweens can't be supported without overriding
 * the UBlueprintAsyncActionBase class).
 */
UCLASS(BlueprintType)
class TWEENMAKER_API UBaseTween : public UObject
{
    GENERATED_BODY()

public:
    UBaseTween(const FObjectInitializer& ObjectInitializer);

    virtual void BeginDestroy() override;

	friend class UTweenManagerComponent;
	friend class UTweenContainer;

    /*
     **************************************************************************
     * "Tween Info" methods
     **************************************************************************
     */

    /**
     * Returns the total duration of the Tween.
     *
     * @return The duration (in seconds).
     */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Current Duration Info"), Category = "Tween|Info")
    float GetTweenDuration()
    {
        return mTweenDuration;
    }

    /**
     * Returns the total time that has passed since the beginning of the Tween.
     *
     * @return The duration (in seconds).
     */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Elapsed Time Info"), Category = "Tween|Info")
    float GetTweenElapsedTime()
    {
        return mElapsedTime;
    }

    /**
     * Returns the Tween's target object.
     *
     * @return The target object.
     */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Target Info"), Category = "Tween|Info")
    UObject* GetTweenTarget()
    {
        return mTargetObject.Get();
    }

    /**
     * Returns the TweenContainer that owns this Tween. It can be used to append/joins new Tweens at the end or to manipulate all owned Tweens.
     *
     * @return The target object.
     */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Container Info"), Category = "Tween|Info")
    UTweenContainer* GetTweenContainer()
    {
        return mOwningTweenContainer;
    }

    /**
     * Checks if the Tween is paused.
     *
     * @return True if the Tween is paused; false otherwise.
     */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Is Paused Info"), Category = "Tween|Info")
    bool IsTweenPaused()
    {
        return bIsTweenPaused;
    }

    /**
     * Checks if this Tween is actually tweening right now.
     *
     * @return True if the Tween is tweening; false otherwise.
     */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Is Tweening Info"), Category = "Tween|Info")
    bool IsTweening()
    {
        return !bIsTweenDone && !bIsTweenPaused;
    }

    /**
     * Retrieves the current timescale.
     *
     * @return The timescale.
     */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Time Scale Info"), Category = "Tween|Info")
    float GetTimeScale()
    {
        return mTimeScale;
    }

    /**
     * Returns the name of the Tween ("None" if it wasn't set).
     *
     * @return The name of the Tween.
     */
    UFUNCTION(BlueprintPure, meta = (KeyWords="Tween Name Info"), Category = "Tween|Info")
    FName GetTweenName()
    {
        return mTweenName;
    }

    /**
     * Sets the name of the Tween. The name can be queried by calling GetTweenName.
     *
     * @param TweenName The name of the Tween.
     */
    UFUNCTION(BlueprintCallable, meta = (KeyWords="Tween Set Name Info"), Category = "Tween|Info")
    void SetTweenName(FName TweenName = NAME_None);

    /*
     **************************************************************************
     * "Tween Utils" methods
     **************************************************************************
     */

    /**
     * Marks this Tween for deletion. The actual deletion will be done in the next Tick.
     *
     * @param FireEndEvent If set, the 'On Tween End' event will be fired before deleting the Tween.
     */
    UFUNCTION(BlueprintCallable, meta = (KeyWords="Tween Delete Remove"), Category = "Tween|Utils")
    void DeleteTween(bool FireEndEvent = false);

    /**
      * Pauses the Tween.
      *
      * @param SkipTween If true, the Tween will be skipped. It's useful when the Tween is part of bigger sequence
      *        and it just need to be paused without blocking the whole sequence.
      */
    UFUNCTION(BlueprintCallable, meta = (KeyWords="Tween Pause"), Category = "Tween|Utils")
    void PauseTween(bool SkipTween = false);

    /**
     * Resumes the execution of this Tween.
     */
    UFUNCTION(BlueprintCallable, meta = (KeyWords="Tween Resume"), Category = "Tween|Utils")
    void ResumeTween();

    /**
      * Toggles pause on/off for this Tween.
      *
      * @param SkipTween If true, the Tween will be skipped when paused. It's useful when the Tween is part of bigger sequence
      *        and it just need to be paused without blocking the whole sequence.
      */
    UFUNCTION(BlueprintCallable, meta = (KeyWords="Tween Toggle Pause Resume"), Category = "Tween|Utils")
    void TogglePauseTween(bool SkipTween = false);

	/**
	 * Restarts this Tween from the beginning. It resumes the Tween, in case it was paused.
	 */
	UFUNCTION(BlueprintCallable, meta = (KeyWords = "Tween Restart"), Category = "Tween|Utils")
	void RestartTween();

    /**
     * Changes the delay before starting the Tween. If the Tween has already started, it won't have any effect.
     *
     * @param NewDelay The new value.
     */
    UFUNCTION(BlueprintCallable, meta = (KeyWords="Set Time Scale Tween"), Category = "Tween|Utils")
    void SetDelay(float NewDelay);

    /**
     * Changes the timescale of the Tween. Higher values will make it faster, lower ones will make it slowers.
     * If the value is < 0 the Tween will go backward.
     *
     * @param NewTimeScale The new value.
     */
    UFUNCTION(BlueprintCallable, meta = (KeyWords="Set Time Scale Tween"), Category = "Tween|Utils")
    void SetTimeScale(float NewTimeScale);

	/**
	 * Sets the CurveFloat to be used instead of the defaults easing types. Make sure that the Curve stays in the [0, 1] 
	 * range both for time and float values for better results.
	 *
	 * @param Curve The CurveFloat that will be used to ease the Tween.
	 */
	UFUNCTION(BlueprintCallable, meta = (KeyWords = "Tween Custom Easing"), Category = "Tween|Utils")
	void SetCustomEasisng(UCurveFloat *Curve);

    /*
     **************************************************************************
     * Methods used internally. Should not be called.
     **************************************************************************
     */


    /**
     * @brief UpdateTween is called at each Tick to update the Tween state.
     *
     * @param pDeltaTime The time passed from the last frame
     * @param pTimeScale The time scale to apply to the Tween when computing the current time
     *
     * @return True if the Tween is done, false otherwise.
     */
    bool UpdateTween(float pDeltaTime, float pTimeScale);

    /**
     * @brief InitBase is in charge of initializing the Tween's parameters.
     */
    void InitBase(UTweenContainer *pTweenContainer, int32 pSequenceIndex, TWeakObjectPtr<UObject> pTargetObject, ETweenTargetType pTargetType,
                  ETweenEaseType pEaseType, float pDuration, float pDelay, float pTimeScale, bool pTweenWhileGameIsPaused, int32 pNumLoops = 1,
                  ETweenLoopType pLoopType = ETweenLoopType::Yoyo);


    /**
     * @brief ShouldInvertTween it's called if the Tween is created with a timescale < 0, thus it needs to be
     *        initialized by inverting the starting/ending values after they're computed in the PrepareTween() method.
     */
    void ShouldInvertTween()
    {
        bShouldInvertTweenFromStart = true;
    }

    /**
     * @brief IsTweenDone checks if the Tween is done.
     *
     * @return True if it's finished, false otherwise.
     */
    bool IsTweenDone()
    {
        return bIsTweenDone;
    }

    /**
     * @brief ShouldSkipTween checks if the Tween is should be skipped.
     *
     * @return True if it's to be skipped, false otherwise.
     */
    bool ShouldSkipTween()
    {
        return bShouldSkipTween;
    }

    /**
     * @brief IsTweenPendingDeletion checks if the Tween was marked to be deleted.
     *
     * @return True if it's to be deleted, false otherwise.
     */
    bool IsTweenPendingDeletion()
    {
        return bIsPendingDeletion;
    }

    /**
     * @brief GetTweenSequenceIndex retrieves the index of this Tween among the sequences it's in.
     *
     * @return The index of the sequence this Tween belongs to.
     */
    int32 GetTweenSequenceIndex()
    {
        return mTweenSequenceIndex;
    }

    /**
     * @brief GetNumLoops retrieves the number of loops this Tween has to do.
     *
     * @return The number of loops.
     */
    int32 GetNumLoops()
    {
        return mNumLoops;
    }

    /**
     * @brief GetNumLoops retrieves the type of loop associated to this Tween.
     *
     * @return The type of loop.
     */
    ETweenLoopType GetLoopType()
    {
        return mLoopType;
    }

    /**
     * @brief GetTweenGenericType retrieves the generic type this Tween belongs to.
     *
     * @return The type.
     */
    ETweenGenericType GetTweenGenericType()
    {
        if(mTweenGenericType == ETweenGenericType::Any)
        {
            mTweenGenericType = Utility::FindOutTypeOfTween(this);
        }

        return mTweenGenericType;
    }


    /*
     **************************************************************************
     * Methods used internally that are overridden by children classes. Should not be called.
     **************************************************************************
     */

    /**
     * @brief PrepareTween is called ONCE in the lifetime of the Tween (even if it loops) and it's the chance to prepare the Tween variables.
     */
    virtual void PrepareTween()
    {

    }

	/**
	 * @brief It brings the Tween to its starting or ending position, without calling an update or firing any delegates.
	 *
	 * @brief pBeginning True if it should position at the beginning; false if it's at the end.
	 */
	virtual void PositionAt(bool pBeginning)
	{

	}

    /**
     * @brief PreDelay is called ONCE in the lifetime of the Tween (even if it loops) and it's the chance to do some work
     *        before a delay starts. The difference between this and PrepareTween() is that PreDelay() is only called
     *        if there actually is a delay on the Tween, and in that case it's called when the delay starts, while
     *        PrepareTween() is called when the delay is over and the Tween is actually starting.
     */
    virtual void PreDelay()
    {
        bHasCalledPreDelay = true;
    }

    /**
     * @brief Restart resets the Tween.
     *
     * @param pRestartFromEnd If true, the Tween will go backward (i.e. Yoyo style).
     */
    virtual void Restart(bool pRestartFromEnd)
    {
        // I don't restart the delay, otherwise it will always apply it at each loop, with is not usually a good thing
        mDelayElapsedTime = 0.0f;
        bIsTweenDone = false;
    }

    /**
     * @brief Inverts the Tween direction. It's called when the timescale changes sign.
     *
     * @param pShouldInvertElapsedTime If true, the elapsed time is inverted. Useful when changing the timescale's sign at runtime,
     *        but shouldn't be done if the Tween starts backward (i.e. with timescale < 0)
     */
    virtual void Invert(bool pShouldInvertElapsedTime = true)
    {
        if(pShouldInvertElapsedTime)
        {
            mElapsedTime = FMath::Abs(mTweenDuration - mElapsedTime);
        }
    }


protected:

    /**
     * @brief Update updates the tween state.
     *
     * @param pDeltaTime The time passed from the last frame
     * @param pTimeScale The time scale to apply to the Tween when computing the current time
     *
     * @return True if the Tween is done, false otherwise.
     */
    virtual bool Update(float pDeltaTime, float pTimeScale)
    {
        return false;
    }

    /**
     * @brief BroadcastOnTweenStart is called when the Tween should broadcast that the Tween started.
     */
    virtual void BroadcastOnTweenStart()
    {

    }

    /**
     * @brief BroadcastOnTweenStart is called when the Tween should broadcast an update of the Tween.
     */
    virtual void BroadcastOnTweenUpdate()
    {

    }

    /**
     * @brief BroadcastOnTweenStart is called when the Tween should broadcast that the Tween ended.
     */
    virtual void BroadcastOnTweenEnd()
    {

    }

    virtual void OnTweenPausedInternal()
    {

    }

    virtual void OnTweenResumedInternal()
    {

    }

    virtual void OnTweenDeletedInternal()
    {

    }

public:

    /*
     **************************************************************************
     * "Append Tween" methods
     **************************************************************************
     */

    /* TweenVector methods */

    /**
      * Appends to this Tween a new one that moves an Actor from its current location (at the start of the Tween) to the given location.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Actor to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Move Actor To", KeyWords="Tween Move Actor To Append Sequence", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|Actor|Move")
    UTweenVector* AppendTweenMoveActorTo(UPARAM(ref) AActor *TweenTarget, FVector To,
                                         float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                         bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                         int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                         float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that moves an Actor by the given offset with respect to its current location (at the start of the Tween).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Actor to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Move Actor By", KeyWords="Tween Move Actor By Append Sequence", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|Actor|Move")
    UTweenVector* AppendTweenMoveActorBy(UPARAM(ref) AActor *TweenTarget, FVector By,
                                         float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                         bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                         int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                         float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that scales an Actor from its current scale (at the start of the Tween) to the given size.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Actor to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Scale Actor To", KeyWords="Tween Scale Actor To Append Sequence", AdvancedDisplay=4), Category = "Tween|StandardTween|Actor|Scale")
    UTweenVector* AppendTweenScaleActorTo(UPARAM(ref) AActor *TweenTarget, FVector To,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                          int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that scales an Actor by the given offset with respect to its current size (at the start of the Tween).
      * It will be executed after this Tween is completed.

      * @param TweenTarget The Actor to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Scale Actor By", KeyWords="Tween Scale Actor By Append Sequence", AdvancedDisplay=4), Category = "Tween|StandardTween|Actor|Scale")
    UTweenVector* AppendTweenScaleActorBy(UPARAM(ref) AActor *TweenTarget, FVector By,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                          int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that moves a SceneComponent from its current location (at the start of the Tween) to the given location.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The SceneComponent to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Move Scene Component To", KeyWords="Tween Move Scene Component To Append Sequence", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Move")
    UTweenVector* AppendTweenMoveSceneComponentTo(UPARAM(ref) USceneComponent *TweenTarget, FVector To,
                                                  float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                  bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                  int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                                  float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that moves a SceneComponent by the given offset with respect to its current location (at the start of the Tween).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The SceneComponent to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Move Scene Component By", KeyWords="Tween Move Scene Component By Append Sequence", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Move")
    UTweenVector* AppendTweenMoveSceneComponentBy(UPARAM(ref) USceneComponent *TweenTarget, FVector By,
                                                  float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                  bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                  int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                                  float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that scales a SceneComponent from its current scale (at the start of the Tween) to the given size.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The SceneComponent to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Scale Scene Component To", KeyWords="Tween Scale Scene Component To Append Sequence", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Scale")
    UTweenVector* AppendTweenScaleSceneComponentTo(UPARAM(ref) USceneComponent *TweenTarget, FVector To,
                                                   float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                   bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                   int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                                   float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that scales a SceneComponent by the given offset with respect to its current size (at the start of the Tween).
      * It will be executed after this Tween is completed.

      * @param TweenTarget The SceneComponent to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Scale Scene Component By", KeyWords="Tween Scale Scene Component By Append Sequence", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Scale")
    UTweenVector* AppendTweenScaleSceneComponentBy(UPARAM(ref) USceneComponent *TweenTarget, FVector By,
                                                   float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                   bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                   int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                                   float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to the TweenContainer a custom Tween that goes from a starting value to an ending one. This type of Tween is meant for creating custom behaviours
      * by binding a function or an event to the OnTweenUpdate delegate in order to use the tweened value at each Tick by calling GetCurrentValue from the Tween object.
      * The Tween will be executed when all Tweens before it are completed.
      *
      * @param TweenTarget The target of the tween. Can be anything, as long as it's not null.
      * @param From The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Custom Vector", KeyWords="Tween Move Append Sequence", AdvancedDisplay=5), Category = "Tween|StandardTween|Custom")
    UTweenVector* AppendTweenCustomVector(UPARAM(ref) UObject *TweenTarget, FVector From, FVector To,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /* TweenRotator methods */

    /**
      * Appends to this Tween a new one that rotates an Actor from its current orientation (at the start of the Tween) to the given rotation.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Actor to rotate.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Rotate Actor To", KeyWords="Tween Rotate Actor To Append Sequence", AdvancedDisplay=4), Category = "Tween|StandardTween|Actor|Rotate")
    UTweenRotator* AppendTweenRotateActorTo(UPARAM(ref) AActor *TweenTarget, FRotator To,
                                            float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                            bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                            int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that rotates an Actor by the given offset with respect to its current orientation (at the start of the Tween).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Actor to rotate.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Rotate Actor By", KeyWords="Tween Rotate Actor By Append Sequence", AdvancedDisplay=4), Category = "Tween|StandardTween|Actor|Rotate")
    UTweenRotator* AppendTweenRotateActorBy(UPARAM(ref) AActor *TweenTarget, FRotator By,
                                            float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                            bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                            int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Appends to this Tween a new one that rotates a SceneComponent from its current orientation (at the start of the Tween) to the given rotation.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The SceneComponent to rotate.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Rotate Scene Component To", KeyWords="Tween Rotate Scene Component To Append Sequence", AdvancedDisplay=4), Category = "Tween|StandardTween|SceneComponent|Rotate")
    UTweenRotator* AppendTweenRotateSceneComponentTo(UPARAM(ref) USceneComponent *TweenTarget,
                                                     FRotator To,
                                                     float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                     bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                     int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                                     float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that rotates a SceneComponent by the given offset with respect to its current orientation (at the start of the Tween).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The SceneComponent to rotate.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Rotate Scene Component By", KeyWords="Tween Rotate Scene Component By Append Sequence", AdvancedDisplay=4), Category = "Tween|StandardTween|SceneComponent|Rotate")
    UTweenRotator* AppendTweenRotateSceneComponentBy(UPARAM(ref) USceneComponent *TweenTarget,
                                                     FRotator By,
                                                     float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                     bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                     int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                                     float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /* TweenLinearColor methods */


    /**
      * Appends to this Tween a new one that modifies a "Vector" material parameter from the starting value to the ending value.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target material.
      * @param ParameterName The name of the material's parameter to tween.
      * @param From	The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Material Vector From To", KeyWords="Tween Tween Material Vector From To Append Sequence", AdvancedDisplay=5), Category = "Tween|StandardTween|Material")
    UTweenLinearColor* AppendTweenMaterialVectorFromTo(UPARAM(ref) UMaterialInstanceDynamic *TweenTarget, FName ParameterName, FLinearColor From, FLinearColor To,
                                                       float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                       int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                                       float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that modifies a "Vector" material parameter from its current value (at the start of the Tween) to the ending value.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target material.
      * @param ParameterName The name of the material's parameter to tween.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Material Vector To", KeyWords="Tween Tween Material Vector To Append Sequence", AdvancedDisplay=4), Category = "Tween|StandardTween|Material")
    UTweenLinearColor* AppendTweenMaterialVectorTo(UPARAM(ref) UMaterialInstanceDynamic *TweenTarget, FName ParameterName, FLinearColor To,
                                                   float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                   int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                                   float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /* TweenFloat methods */


    /**
      * Appends to this Tween a new one that modifies a "float" material parameter from the starting value to the ending value.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target material.
      * @param ParameterName The name of the material's parameter to tween.
      * @param From	The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Material Float From To", KeyWords="Tween Tween Material Float From To Append Sequence", AdvancedDisplay=5), Category = "Tween|StandardTween|Material")
    UTweenFloat* AppendTweenMaterialFloatFromTo(UPARAM(ref) UMaterialInstanceDynamic *TweenTarget, FName ParameterName, float From, float To,
                                                float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                                float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that modifies a "float" material parameter from its current value (at the start of the Tween) to the ending value.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target material.
      * @param ParameterName The name of the material's parameter to tween.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Material Float To", KeyWords="Tween Tween Material Float To Append Sequence", AdvancedDisplay=4), Category = "Tween|StandardTween|Material")
    UTweenFloat* AppendTweenMaterialFloatTo(UPARAM(ref) UMaterialInstanceDynamic *TweenTarget, FName ParameterName, float To,
                                            float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                            int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);



    /**
      * Appends to this Tween a new one that rotates an Actor around a given pivot point, with a given radius and rotation axis. The Actor will move
      * relative to its parent (if there is none, it will move in world coordinates).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target Actor.
      * @param PivotPoint The point around which to rotate.
      * @param StartingAngle The starting angle of the rotation (in degrees).
      * @param EndingAngle The ending angle of the rotation (in degrees).
      * @param Radius The radius of the rotation, in relation to the pivot point.
      * @param Axis The axis around which to rotate.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Rotate Actor Around Point", KeyWords="Tween Rotate Actor Around Point Append Sequence", AdvancedDisplay=7), Category = "Tween|StandardTween|Actor|Rotate")
    UTweenFloat* AppendTweenRotateActorAroundPoint(UPARAM(ref) AActor *TweenTarget, FVector PivotPoint, float StartingAngle, float EndingAngle, float Radius,
                                                   FVector Axis = FVector(0.0f, 0.0f, 1.0f),
                                                   float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                   bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                   int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Restart,
                                                   float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to the TweenContainer a Tween that rotates an Actor around a given pivot point by an offset angle. The radius and the rotation axis will be automatically computed.
      * The Actor will move relative to its parent (if there is none, it will move in world coordinates).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target Actor.
      * @param PivotPoint The point around which to rotate.
      * @param OffsetAngle The ending angle of the rotation (in degrees).
      * @param ReferenceAxis The reference axis used to compute the direction of the rotation. Different axes will result in different kind of rotations: experiment to find the one you need.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Rotate Actor Around Point By Offset", KeyWords="Tween Rotate Actor Around Point By Offset Append Sequence", AdvancedDisplay=6), Category = "Tween|StandardTween|Actor|Rotate")
    UTweenFloat* AppendTweenRotateActorAroundPointByOffset(UPARAM(ref) AActor *TweenTarget, FVector PivotPoint, float OffsetAngle,
                                                           ETweenReferenceAxis ReferenceAxis,
                                                           float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                           bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                           int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Restart,
                                                           float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that makes an Actor follow a spline path.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target Actor.
      * @param Spline The SplineComponent path to follow.
      * @param Duration The total duration of the Tween (in seconds).
      * @param ApplyRotation If set, the spline rotation will be applied to the Actor.
      * @param ApplyScale If set, the spline scale will be applied to the Actor.
      * @param UseConstantSpeed If set, the movement will be at constant speed across the spline points (but it will sill take into account the easing).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Actor Follow Spline", KeyWords="Tween Actor Follow Spline Append Sequence", AdvancedDisplay=7, HidePin=SequenceIndex), Category = "Tween|StandardTween|Actor|Spline")
    UTweenFloat* AppendTweenActorFollowSpline(UPARAM(ref) AActor *TweenTarget, USplineComponent *Spline,
                                              float Duration = 1.0f, bool ApplyRotation = false, bool ApplyScale = false,
                                              bool UseConstantSpeed = true, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                              bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                              int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                              float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Appends to this Tween a new one that rotates an SceneComponent around a given pivot point, with a given radius and rotation axis. The Actor will move
      * relative to its parent (if there is none, it will move in world coordinates).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target SceneComponent.
      * @param PivotPoint The point around which to rotate.
      * @param StartingAngle The starting angle of the rotation (in degrees).
      * @param EndingAngle The ending angle of the rotation (in degrees).
      * @param Radius The radius of the rotation, in relation to the pivot point.
      * @param Axis The axis around which to rotate.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Rotate Scene Component Around Point", KeyWords="Tween Rotate Scene Component Around Point Append Sequence", AdvancedDisplay=7), Category = "Tween|StandardTween|SceneComponent|Rotate")
    UTweenFloat* AppendTweenRotateSceneComponentAroundPoint(UPARAM(ref) USceneComponent *TweenTarget, FVector PivotPoint, float StartingAngle, float EndingAngle, float Radius,
                                                            FVector Axis = FVector(0.0f, 0.0f, 1.0f),
                                                            float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                            bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                            int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Restart,
                                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Appends to the TweenContainer a Tween that rotates an SceneComponent around a given pivot point by an offset angle. The radius and the rotation axis will be automatically computed. The SceneComponent will move
      * relative to its parent (if there is none, it will move in world coordinates).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target SceneComponent.
      * @param PivotPoint The point around which to rotate.
      * @param OffsetAngle The ending angle of the rotation (in degrees).
      * @param ReferenceAxis The reference axis used to compute the direction of the rotation. Different axes will result in different kind of rotations: experiment to find the one you need.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Rotate Scene Component Around Point By Offset", KeyWords="Tween Rotate Scene Component Around Point By Offset Append Sequence", AdvancedDisplay=6), Category = "Tween|StandardTween|SceneComponent|Rotate")
    UTweenFloat* AppendTweenRotateSceneComponentAroundPointByOffset(UPARAM(ref) USceneComponent *TweenTarget, FVector PivotPoint, float OffsetAngle,
                                                                    ETweenReferenceAxis ReferenceAxis,
                                                                    float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                                    bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                                    int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Restart,
                                                                    float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that makes an SceneComponent follow a spline path.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target SceneComponent.
      * @param Spline The SplineComponent path to follow.
      * @param Duration The total duration of the Tween (in seconds).
      * @param ApplyRotation If set, the spline rotation will be applied to the SceneComponent.
      * @param ApplyScale If set, the spline scale will be applied to the SceneComponent.
      * @param UseConstantSpeed If set, the movement will be at constant speed across the spline points (but it will sill take into account the easing).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Scene Component Follow Spline", KeyWords="Tween Scene Component Follow Spline Append Sequence", AdvancedDisplay=7, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Spline")
    UTweenFloat* AppendTweenSceneComponentFollowSpline(UPARAM(ref) USceneComponent *TweenTarget, USplineComponent *Spline,
                                                       float Duration = 1.0f, bool ApplyRotation = false, bool ApplyScale = false,
                                                       bool UseConstantSpeed = true, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                       bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                       int32 NumLoops = 1, ETweenLoopType mLoopType = ETweenLoopType::Yoyo,
                                                       float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Appends to this Tween a new one that makes a UMG widget rotate to the given angle.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target Widget.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Widget Angle To", KeyWords="Tween Widget Angle To UMG Sequence", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Angle")
    UTweenFloat* AppendTweenWidgetAngleTo(UPARAM(ref) UWidget *TweenTarget, float To,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that makes a UMG widget's opacity reach the given value.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target Widget.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Widget Opacity To", KeyWords="Tween Widget Opacity To UMG Sequence", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Opacity")
    UTweenFloat* AppendTweenWidgetOpacityTo(UPARAM(ref) UWidget *TweenTarget, float To,
                                            float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                            int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Appends to the TweenContainer a custom Tween that goes from a starting float value to an ending one. This type of Tween is meant for creating custom behaviours
      * by binding a function or an event to the OnTweenUpdate delegate in order to use the tweened value at each Tick by calling GetCurrentValue from the Tween object.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The target of the tween. Can be anything, as long as it's not null.
      * @param From The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Custom Float", KeyWords="Tween Float Append Sequence", AdvancedDisplay=5), Category = "Tween|StandardTween|Custom")
    UTweenFloat* AppendTweenCustomFloat(UPARAM(ref) UObject *TweenTarget, float From, float To,
                                        float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                        int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                        float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /* TweenVector2D methods */

    /**
      * Appends to this Tween a new one that moves a UMG widget from its current location (at the start of the Tween) to the given location.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Widget to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Move Widget To", KeyWords="Tween Move Widget UMG To Append Sequence", AdvancedDisplay=6, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Move")
    UTweenVector2D* AppendTweenMoveWidgetTo(UPARAM(ref) UWidget *TweenTarget, FVector2D To,
                                            float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                            int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that moves a UMG widget by the given offset with respect to its current location (at the start of the Tween).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Widget to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Move Widget By", KeyWords="Tween Move Widget UMG By Append Sequence", AdvancedDisplay=6, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Move")
    UTweenVector2D* AppendTweenMoveWidgetBy(UPARAM(ref) UWidget *TweenTarget, FVector2D By,
                                            float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                            int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Appends to this Tween a new one that moves a UMG widget from its current scale (at the start of the Tween) to the given size.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Widget to scale.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Scale Widget To", KeyWords="Tween Scale Widget UMG To Append Sequence", AdvancedDisplay=5, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Scale")
    UTweenVector2D* AppendTweenScaleWidgetTo(UPARAM(ref) UWidget *TweenTarget, FVector2D To,
                                             float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                             int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                             float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Appends to this Tween a new one that scales a UMG widget by the given offset with respect to its current size (at the start of the Tween).
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Widget to scale.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Scale Widget By", KeyWords="Tween Scale Widget UMG By Append Sequence", AdvancedDisplay=5, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Scale")
    UTweenVector2D* AppendTweenScaleWidgetBy(UPARAM(ref) UWidget *TweenTarget, FVector2D By,
                                             float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                             int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                             float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to this Tween a new one that modifies a UMG widget's shear value from its current value (at the start of the Tween) to the given ending value.
      * It will be executed after this Tween is completed.
      *
      * @param TweenTarget The Widget to shear.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Shear Widget To", KeyWords="Tween Shear Widget UMG To Append Sequence", AdvancedDisplay=5, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Shear")
    UTweenVector2D* AppendTweenShearWidgetTo(UPARAM(ref) UWidget *TweenTarget, FVector2D By,
                                             float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                             int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                             float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Appends to the TweenContainer a custom Tween that goes from a starting value to an ending one. This type of Tween is meant for creating custom behaviours
      * by binding a function or an event to the OnTweenUpdate delegate in order to use the tweened value at each Tick by calling GetCurrentValue from the Tween object.
      * The Tween will be executed when all Tweens before it are completed.
      *
      * @param TweenTarget The target of the tween. Can be anything, as long as it's not null.
      * @param From The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param NumLoops	The number of times the Tween should repeat. If <= 0, it will loop indefinitely.
      * @param LoopType	The type of loop to apply (if "yoyo", it will go backwards when it reached the end; otherwise it will reset).
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Append Tween Custom Vector2D", KeyWords="Tween Custom Vector2D Append Sequence", AdvancedDisplay=5, HidePin=SequenceIndex), Category = "Tween|StandardTween|Custom")
    UTweenVector2D* AppendTweenCustomVector2D(UPARAM(ref) UObject *TweenTarget, FVector2D From, FVector2D To,
                                              float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                              int32 mNumLoops = 1, ETweenLoopType LoopType = ETweenLoopType::Yoyo,
                                              float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /*
     **************************************************************************
     * "Join Tween" methods
     **************************************************************************
     */


    /* TweenVector methods */

    /**
      * Joins to this Tween a new one that moves an Actor by the given offset with respect to its current location (at the start of the Tween).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Actor to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Move Actor To", KeyWords="Tween Move Actor To Join Parallel", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|Actor|Move")
    UTweenVector* JoinTweenMoveActorTo(UPARAM(ref) AActor *TweenTarget, FVector To,
                                       float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                       bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                       float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that moves an Actor by the given offset with respect to its current location (at the start of the Tween).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Actor to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Move Actor By", KeyWords="Tween Move Actor By Join Parallel", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|Actor|Move")
    UTweenVector* JoinTweenMoveActorBy(UPARAM(ref) AActor *TweenTarget, FVector By,
                                       float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                       bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                       float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that scales an Actor from its current scale (at the start of the Tween) to the given size.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Actor to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Scale Actor To", KeyWords="Tween Scale Actor To Join Parallel", AdvancedDisplay=4), Category = "Tween|StandardTween|Actor|Scale")
    UTweenVector* JoinTweenScaleActorTo(UPARAM(ref) AActor *TweenTarget, FVector To,
                                        float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                        bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                        float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that scales an Actor by the given offset with respect to its current size (at the start of the Tween).
      * It will be executed in parallel with this Tween.

      * @param TweenTarget The Actor to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Scale Actor By", KeyWords="Tween Scale Actor By Join Parallel", AdvancedDisplay=4), Category = "Tween|StandardTween|Actor|Scale")
    UTweenVector* JoinTweenScaleActorBy(UPARAM(ref) AActor *TweenTarget, FVector By,
                                        float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                        bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                        float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that moves a SceneComponent by the given offset with respect to its current location (at the start of the Tween).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The SceneComponent to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Move Scene Component To", KeyWords="Tween Move Scene Component To Join Parallel", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Move")
    UTweenVector* JoinTweenMoveSceneComponentTo(UPARAM(ref) USceneComponent *TweenTarget, FVector To,
                                                float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that moves a SceneComponent by the given offset with respect to its current location (at the start of the Tween).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The SceneComponent to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Move Scene Component By", KeyWords="Tween Move Scene Component By Join Parallel", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Move")
    UTweenVector* JoinTweenMoveSceneComponentBy(UPARAM(ref) USceneComponent *TweenTarget, FVector By,
                                                float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that scales a SceneComponent from its current scale (at the start of the Tween) to the given size.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The SceneComponent to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Scale Scene Component To", KeyWords="Tween Scale Scene Component To Join Parallel", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Scale")
    UTweenVector* JoinTweenScaleSceneComponentTo(UPARAM(ref) USceneComponent *TweenTarget, FVector To,
                                                 float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                 bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                 float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that scales a SceneComponent by the given offset with respect to its current size (at the start of the Tween).
      * It will be executed in parallel with this Tween.

      * @param TweenTarget The SceneComponent to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Scale Scene Component By", KeyWords="Tween Scale Scene Component By Join Parallel", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Scale")
    UTweenVector* JoinTweenScaleSceneComponentBy(UPARAM(ref) USceneComponent *TweenTarget, FVector By,
                                                 float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                 bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                 float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to the TweenContainer a custom Tween that goes from a starting value to an ending one. This type of Tween is meant for creating custom behaviours
      * by binding a function or an event to the OnTweenUpdate delegate in order to use the tweened value at each Tick by calling GetCurrentValue from the Tween object.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target of the tween. Can be anything, as long as it's not null.
      * @param From The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Custom Vector", KeyWords="Tween Custom Vector Join Parallel", AdvancedDisplay=5), Category = "Tween|StandardTween|Custom")
    UTweenVector* JoinTweenCustomVector(UPARAM(ref) UObject *TweenTarget, FVector From, FVector To,
                                        float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                        float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /* TweenRotator methods */


    /**
      * Joins to this Tween a new one that rotates an Actor from its current orientation (at the start of the Tween) to the given rotation.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Actor to rotate.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Rotate Actor To", KeyWords="Tween Rotate Actor To Join Parallel", AdvancedDisplay=4), Category = "Tween|StandardTween|Actor|Rotate")
    UTweenRotator* JoinTweenRotateActorTo(UPARAM(ref) AActor *TweenTarget, FRotator To,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that rotates an Actor by the given offset with respect to its current orientation (at the start of the Tween).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Actor to rotate.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Rotate Actor By", KeyWords="Tween Rotate Actor By Join Parallel", AdvancedDisplay=4), Category = "Tween|StandardTween|Actor|Rotate")
    UTweenRotator* JoinTweenRotateActorBy(UPARAM(ref) AActor *TweenTarget, FRotator By,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that rotates a SceneComponent from its current orientation (at the start of the Tween) to the given rotation.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The SceneComponent to rotate.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Rotate Scene Component To", KeyWords="Tween Rotate Scene Component To Join Parallel", AdvancedDisplay=4), Category = "Tween|StandardTween|SceneComponent|Rotate")
    UTweenRotator* JoinTweenRotateSceneComponentTo(UPARAM(ref) USceneComponent *TweenTarget,
                                                   FRotator To,
                                                   float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                   bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                   float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that rotates a SceneComponent by the given offset with respect to its current orientation (at the start of the Tween).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The SceneComponent to rotate.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Rotate Scene Component By", KeyWords="Tween Rotate Scene Component To Join Parallel", AdvancedDisplay=4), Category = "Tween|StandardTween|SceneComponent|Rotate")
    UTweenRotator* JoinTweenRotateSceneComponentBy(UPARAM(ref) USceneComponent *TweenTarget,
                                                   FRotator By,
                                                   float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                   bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                   float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /* TweenLinearColor methods */

    /**
      * Joins to this Tween a new one that modifies a "Vector" material parameter from the starting value to the ending value.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target material.
      * @param ParameterName The name of the material's parameter to tween.
      * @param From	The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Material Vector From To", KeyWords="Tween Material Vector From To Join Parallel", AdvancedDisplay=5), Category = "Tween|StandardTween|Material")
    UTweenLinearColor* JoinTweenMaterialVectorFromTo(UPARAM(ref) UMaterialInstanceDynamic *TweenTarget, FName ParameterName, FLinearColor From, FLinearColor To,
                                                     float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                     float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that modifies a "Vector" material parameter from its current value (at the start of the Tween) to the ending value.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target material.
      * @param ParameterName The name of the material's parameter to tween.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Material Vector To", KeyWords="Tween Material Vector To Join Parallel", AdvancedDisplay=4), Category = "Tween|StandardTween|Material")
    UTweenLinearColor* JoinTweenMaterialVectorTo(UPARAM(ref) UMaterialInstanceDynamic *TweenTarget, FName ParameterName, FLinearColor To,
                                                 float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                 float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /* TweenFloat methods */

    /**
      * Joins to this Tween a new one that modifies a "float" material parameter from the starting value to the ending value.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target material.
      * @param ParameterName The name of the material's parameter to tween.
      * @param From	The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Material Float From To", KeyWords="Tween Material Float From To Join Parallel", AdvancedDisplay=5), Category = "Tween|StandardTween|Material")
    UTweenFloat* JoinTweenMaterialFloatFromTo(UPARAM(ref) UMaterialInstanceDynamic *TweenTarget, FName ParameterName, float From, float To,
                                              float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                              float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that modifies a "float" material parameter from its current value (at the start of the Tween) to the ending value.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target material.
      * @param ParameterName The name of the material's parameter to tween.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Material Float To", KeyWords="Tween Material Float To Join Parallel", AdvancedDisplay=4), Category = "Tween|StandardTween|Material")
    UTweenFloat* JoinTweenMaterialFloatTo(UPARAM(ref) UMaterialInstanceDynamic *TweenTarget, FName ParameterName, float To,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that rotates an Actor around a given pivot point, with a given radius and rotation axis. The Actor will move
      * relative to its parent (if there is none, it will move in world coordinates).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target Actor.
      * @param PivotPoint The point around which to rotate.
      * @param StartingAngle The starting angle of the rotation (in degrees).
      * @param EndingAngle The ending angle of the rotation (in degrees).
      * @param Radius The radius of the rotation, in relation to the pivot point.
      * @param Axis The axis around which to rotate.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Rotate Actor Around Point", KeyWords="Tween Rotate Actor Around Point Join Parallel", AdvancedDisplay=7), Category = "Tween|StandardTween|Actor|Rotate")
    UTweenFloat* JoinTweenRotateActorAroundPoint(UPARAM(ref) AActor *TweenTarget, FVector PivotPoint, float StartingAngle, float EndingAngle, float Radius,
                                                 FVector Axis = FVector(0.0f, 0.0f, 1.0f),
                                                 float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                 bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                 float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to the TweenContainer a Tween that rotates an Actor around a given pivot point by an offset angle. The radius and the rotation axis will be automatically computed. The Actor will move
      * relative to its parent (if there is none, it will move in world coordinates).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target Actor.
      * @param PivotPoint The point around which to rotate.
      * @param OffsetAngle The ending angle of the rotation (in degrees).
      * @param ReferenceAxis The reference axis used to compute the direction of the rotation. Different axes will result in different kind of rotations: experiment to find the one you need.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Rotate Actor Around Point By Offset", KeyWords="Tween Rotate Actor Around Point By Offset Join Parallel", AdvancedDisplay=6), Category = "Tween|StandardTween|Actor|Rotate")
    UTweenFloat* JoinTweenRotateActorAroundPointByOffset(UPARAM(ref) AActor *TweenTarget, FVector PivotPoint, float OffsetAngle,
                                                         ETweenReferenceAxis ReferenceAxis,
                                                         float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                         bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                         float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that makes an Actor follow a spline path.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target Actor.
      * @param Spline The SplineComponent path to follow.
      * @param Duration The total duration of the Tween (in seconds).
      * @param ApplyRotation If set, the spline rotation will be applied to the Actor.
      * @param ApplyScale If set, the spline scale will be applied to the Actor.
      * @param UseConstantSpeed If set, the movement will be at constant speed across the spline points (but it will sill take into account the easing).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Actor Follow Spline", KeyWords="Tween Actor Follow Spline Join Parallel", AdvancedDisplay=7, HidePin=SequenceIndex), Category = "Tween|StandardTween|Actor|Spline")
    UTweenFloat* JoinTweenActorFollowSpline(UPARAM(ref) AActor *TweenTarget, USplineComponent *Spline,
                                            float Duration = 1.0f, bool ApplyRotation = false, bool ApplyScale = false,
                                            bool UseConstantSpeed = true, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                            bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that rotates an SceneComponent around a given pivot point, with a given radius and rotation axis. The Actor will move
      * relative to its parent (if there is none, it will move in world coordinates).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target SceneComponent.
      * @param PivotPoint The point around which to rotate.
      * @param StartingAngle The starting angle of the rotation (in degrees).
      * @param EndingAngle The ending angle of the rotation (in degrees).
      * @param Radius The radius of the rotation, in relation to the pivot point.
      * @param Axis The axis around which to rotate.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Rotate Scene Component Around Point", KeyWords="Tween Rotate Scene Component Around Point Join Parallel", AdvancedDisplay=7), Category = "Tween|StandardTween|SceneComponent|Rotate")
    UTweenFloat* JoinTweenRotateSceneComponentAroundPoint(UPARAM(ref) USceneComponent *TweenTarget, FVector PivotPoint, float StartingAngle, float EndingAngle, float Radius,
                                                          FVector Axis = FVector(0.0f, 0.0f, 1.0f),
                                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                          bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);



    /**
      * Joins to the TweenContainer a Tween that rotates an SceneComponent around a given pivot point by an offset angle. The radius and the rotation axis will be automatically computed. The SceneComponent will move
      * relative to its parent (if there is none, it will move in world coordinates).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target SceneComponent.
      * @param PivotPoint The point around which to rotate.
      * @param OffsetAngle The ending angle of the rotation (in degrees).
      * @param ReferenceAxis The reference axis used to compute the direction of the rotation. Different axes will result in different kind of rotations: experiment to find the one you need.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Rotate Scene Component Around Point By Offset", KeyWords="Tween Rotate Scene Component Around Point By Offset Join Parallel", AdvancedDisplay=6), Category = "Tween|StandardTween|SceneComponent|Rotate")
    UTweenFloat* JoinTweenRotateSceneComponentAroundPointByOffset(UPARAM(ref) USceneComponent *TweenTarget, FVector PivotPoint, float OffsetAngle,
                                                                  ETweenReferenceAxis ReferenceAxis,
                                                                  float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                                  bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                                  float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that makes an SceneComponent follow a spline path.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target SceneComponent.
      * @param Spline The SplineComponent path to follow.
      * @param Duration The total duration of the Tween (in seconds).
      * @param ApplyRotation If set, the spline rotation will be applied to the SceneComponent.
      * @param ApplyScale If set, the spline scale will be applied to the SceneComponent.
      * @param UseConstantSpeed If set, the movement will be at constant speed across the spline points (but it will sill take into account the easing).
      * @param EaseType	The easing functions to apply.
      * @param DeleteTweenOnHit If true the Tween will be deleted when the TweenTarget hits something.
      * @param DeleteTweenOnOverlap If true the Tween will be deleted when the TweenTarget overlaps something.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Scene Component Follow Spline", KeyWords="Tween Scene Component Follow Spline Join Parallel", AdvancedDisplay=7, HidePin=SequenceIndex), Category = "Tween|StandardTween|SceneComponent|Spline")
    UTweenFloat* JoinTweenSceneComponentFollowSpline(UPARAM(ref) USceneComponent *TweenTarget, USplineComponent *Spline,
                                                     float Duration = 1.0f, bool ApplyRotation = false, bool ApplyScale = false,
                                                     bool UseConstantSpeed = true, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                                     bool DeleteTweenOnHit = false, bool DeleteTweenOnOverlap = false,
                                                     float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that makes a UMG widget rotate to the given angle.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target Widget.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Widget Angle To", KeyWords="Tween Widget Angle To UMG Join Parallel", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Angle")
    UTweenFloat* JoinTweenWidgetAngleTo(UPARAM(ref) UWidget *TweenTarget, float To,
                                        float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                        float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that makes a UMG widget's opacity reach the given value.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target Widget.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Widget Opacity To", KeyWords="Tween Widget Opacity To UMG Join Parallel", AdvancedDisplay=4, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Opacity")
    UTweenFloat* JoinTweenWidgetOpacityTo(UPARAM(ref) UWidget *TweenTarget, float To,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);



    /**
      * Joins to the TweenContainer a custom Tween that goes from a starting float value to an ending one. This type of Tween is meant for creating custom behaviours
      * by binding a function or an event to the OnTweenUpdate delegate in order to use the tweened value at each Tick by calling GetCurrentValue from the Tween object.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The target of the tween. Can be anything, as long as it's not null.
      * @param From The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Custom Float", KeyWords="Tween Custom Float Join Parallel", AdvancedDisplay=5), Category = "Tween|StandardTween|Custom")
    UTweenFloat* JoinTweenCustomFloat(UPARAM(ref) UObject *TweenTarget, float From, float To,
                                      float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                      float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /* TweenVector2D methods */


    /**
      * Joins to this Tween a new one that moves a UMG widget from its current location (at the start of the Tween) to the given location.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Widget to move.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Move Widget To", KeyWords="Tween Move Widget UMG To Join Parallel", AdvancedDisplay=6, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Move")
    UTweenVector2D* JoinTweenMoveWidgetTo(UPARAM(ref) UWidget *TweenTarget, FVector2D To,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that moves a UMG widget by the given offset with respect to its current location (at the start of the Tween).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Widget to move.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Move Widget By", KeyWords="Tween Move Widget UMG By Join Parallel", AdvancedDisplay=6, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Move")
    UTweenVector2D* JoinTweenMoveWidgetBy(UPARAM(ref) UWidget *TweenTarget, FVector2D By,
                                          float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                          float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

    /**
      * Joins to this Tween a new one that moves a UMG widget from its current scale (at the start of the Tween) to the given size.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Widget to scale.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Scale Widget To", KeyWords="Tween Scale Widget UMG To Join Parallel", AdvancedDisplay=5, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Scale")
    UTweenVector2D* JoinTweenScaleWidgetTo(UPARAM(ref) UWidget *TweenTarget, FVector2D To,
                                           float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                           float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that scales a UMG widget by the given offset with respect to its current size (at the start of the Tween).
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Widget to scale.
      * @param By The offset to apply.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Scale Widget By", KeyWords="Tween Scale Widget UMG By Join Parallel", AdvancedDisplay=5, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Scale")
    UTweenVector2D* JoinTweenScaleWidgetBy(UPARAM(ref) UWidget *TweenTarget, FVector2D By,
                                           float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                           float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to this Tween a new one that modifies a UMG widget's shear value from its current value (at the start of the Tween) to the given ending value.
      * It will be executed in parallel with this Tween.
      *
      * @param TweenTarget The Widget to shear.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Shear Widget To", KeyWords="Tween Shear Widget UMG To Join Parallel", AdvancedDisplay=5, HidePin=SequenceIndex), Category = "Tween|StandardTween|UMG|Shear")
    UTweenVector2D* JoinTweenShearWidgetTo(UPARAM(ref) UWidget *TweenTarget, FVector2D By,
                                           float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                           float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);


    /**
      * Joins to the TweenContainer a custom Tween that goes from a starting value to an ending one. This type of Tween is meant for creating custom behaviours
      * by binding a function or an event to the OnTweenUpdate delegate in order to use the tweened value at each Tick by calling GetCurrentValue from the Tween object.
      * The Tween will be executed when all Tweens before it are completed.
      *
      * @param TweenTarget The target of the tween. Can be anything, as long as it's not null.
      * @param From The starting value.
      * @param To The ending value.
      * @param Duration The total duration of the Tween (in seconds).
      * @param EaseType	The easing functions to apply.
      * @param Delay The initial delay to apply to the Tween (in seconds).
      * @param TimeScale The scale to apply to the Tween execution. If < 1 the Tween will slow down, if > 1 it will speed up. If < 0 it will make the Tween go backward.
	  * @param TweenWhileGameIsPaused Sets whether the Tween should keep updating while the game is paused. Useful for Tween on UMG widgets that animate menus.
	  *
      * @return The newly created Tween.
      */
    UFUNCTION(BlueprintCallable, meta = (DisplayName="Join Tween Custom Vector2D", KeyWords="Tween Custom Vector2D Join Parallel", AdvancedDisplay=5, HidePin=SequenceIndex), Category = "Tween|StandardTween|Custom")
    UTweenVector2D* JoinTweenCustomVector2D(UPARAM(ref) UObject *TweenTarget, FVector2D From, FVector2D To,
                                            float Duration = 1.0f, ETweenEaseType EaseType = ETweenEaseType::Linear,
                                            float Delay = 0.0f, float TimeScale = 1.0f, bool TweenWhileGameIsPaused = false);

protected:

    /** Optional name of the Tween */
    FName mTweenName;

    /** Total duration of the Tween (in seconds) */
    float mTweenDuration;

    /** Elapsed time (in seconds) */
    float mElapsedTime;

    /** Tween target object */
    TWeakObjectPtr<UObject> mTargetObject;

    /** Tween target type */
    ETweenTargetType mTargetType;

    /** Easing equation to use */
    ETweenEaseType mEaseType;

    /** Number of loops to execute (if <= 0 it will loop forever) */
    int32 mNumLoops;

    /** Loop type to use */
    ETweenLoopType mLoopType;

    /** Time scale to apply to the computation of the elapsed time */
    float mTimeScale;

    /** Absolute value of the initial time scale that was set */
    float mTimeScaleAbsolute;

	/** If set (after the Tween creation), this CurveFloat will be used to compute the easing instead of the default easing functions */
	UCurveFloat* mCurveFloat;

	/** If true, the Tween will keep updating even if the game is paused (by default it's false) */
	bool bTweenWhileGameIsPaused;

	/** True if the Tween's PrepareTween() method was already called */
	bool bHasPreparedTween;

private:

    /**
     * @brief PreDestroy is called right before the Tween object gets deleted, for any reason.
     */
    void PreDestroy();

    /** Reference to the TweenContainer that owns this Tween */
    UPROPERTY()
    UTweenContainer *mOwningTweenContainer;

    /** The generic type this Tween belongs to */
    ETweenGenericType mTweenGenericType;

    /** Index of the sequence this Tween belongs to (among the sequences in its owning TweenContainer) */
    int32 mTweenSequenceIndex;

    /** Delay to apply to Tween before starting it. It's applied only once, so if it loops it won't be executed again */
    float mDelay;

    /** Time passed while in delay (in seconds).*/
    float mDelayElapsedTime;

    /** True if the Tween's PreDelay() method was already called. Can only happen if the Tween has a delay */
    bool bHasCalledPreDelay;

    /** True if the Tween was inverted from the start (i.e. the timescale passed was < 0) */
    bool bShouldInvertTweenFromStart;

    // Various self explanatory bools
    bool bIsTweenDone;
    bool bIsTweenPaused;
    bool bShouldSkipTween;
    bool bIsPendingDeletion;

    /**
     * @brief Signal emitted when the name of the Tween changes.
     */
    FTweenNameChanged OnNameChanged;

    /**
     * @brief Signal emitted right before the Tween is destroyed.
     */
    FOnTweenDestroyed OnTweenDestroyed;

};
