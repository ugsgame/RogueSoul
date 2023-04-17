// Copyright 2018 Francesco Desogus. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utils/TweenEnums.h"

class UBaseTween;

// Custom log declaration
DECLARE_LOG_CATEGORY_EXTERN(LogTweenMaker, Log, All);

/**
 * @brief The Utility class implements some utility static methods used in some parts of the plugin
 */
class TWEENMAKER_API Utility
{
public:

    /**
     * @brief DestroyUObject completely destroys a UObject
     *
     * @param pObjectToDestroy the object to destroy
     */
    static void DestroyUObject(UObject *pObjectToDestroy);

    /** @brief Helper methods that convert a given tween type to a more
     *  general tween category (i.e. a "vector scale" tween to a generic "scale" tween)
     */
    static ETweenGenericType ConvertTweenTypeEnum(ETweenVectorType pTweenType);
    static ETweenGenericType ConvertTweenTypeEnum(ETweenVector2DType pTweenType);
    static ETweenGenericType ConvertTweenTypeEnum(ETweenRotatorType pTweenType);
    static ETweenGenericType ConvertTweenTypeEnum(ETweenLinearColorType pTweenType);
    static ETweenGenericType ConvertTweenTypeEnum(ETweenFloatType pTweenType);

    static FVector FromReferenceAxisToVector(ETweenReferenceAxis pReferenceAxis);

    /**
     * @brief Utility method that computes the axis and radius to be used for "RotateAroundPointByOffset" type of Tweens.
     *
     * @param pActor                the target actor
     * @param pPivotPoint           the pivot point
     * @param pReferenceAxis        the reference axis
     * @param pOutAxis              (out) the computed axis
     * @param pOutRadius            (out) the computed radius
     */
    static void ComputeDataForRotateAroundPoint(AActor *pActor, const FVector& pPivotPoint, ETweenReferenceAxis pReferenceAxis, FVector& pOutAxis, float& pOutRadius);

    /**
     * @brief Utility method that computes the axis and radius to be used for "RotateAroundPointByOffset" type of Tweens.
     *
     * @param pComponent            the target component
     * @param pPivotPoint           the pivot point
     * @param pReferenceAxis        the reference axis
     * @param pOutAxis              (out) the computed axis
     * @param pOutRadius            (out) the computed radius
     */
    static void ComputeDataForRotateAroundPoint(USceneComponent *pComponent, const FVector& pPivotPoint, ETweenReferenceAxis pReferenceAxis, FVector& pOutAxis, float& pOutRadius);


    /** @brief Helper method that given a tween it finds the generic tween category it belongs to */
    static ETweenGenericType FindOutTypeOfTween(UBaseTween *pTween);

	/**
	 * @brief Helper function that converts a value from an old range to a new one.
	 */
	static float MapToRange(float pValue, float pOldMin, float pOldMax, float pNewMin, float pNewMax);
};
