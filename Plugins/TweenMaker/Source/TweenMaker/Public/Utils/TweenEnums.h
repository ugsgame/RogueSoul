// Copyright 2018 Francesco Desogus. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "TweenEnums.generated.h"


UENUM()
enum class ETweenEaseType : uint8 {
    Linear,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseOutInQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseOutInCubic,
    EaseInQuart,
    EaseOutQuart,
    EaseInOutQuart,
    EaseOutInQuart,
    EaseInQuint,
    EaseOutQuint,
    EaseInOutQuint,
    EaseOutInQuint,
    EaseInSine,
    EaseOutSine,
    EaseInOutSine,
    EaseOutInSine,
    EaseInExpo,
    EaseOutExpo,
    EaseInOutExpo,
    EaseOutInExpo,
    EaseInCirc,
    EaseOutCirc,
    EaseInOutCirc,
    EaseOutInCirc,
    EaseInElastic,
    EaseOutElastic,
    EaseInOutElastic,
    EaseOutInElastic,
    EaseInBack,
    EaseOutBack,
    EaseInOutBack,
    EaseOutInBack,
    EaseInBounce,
    EaseOutBounce,
    EaseInOutBounce,
    EaseOutInBounce
};

UENUM()
enum class ETweenLoopType : uint8 {
    Yoyo UMETA(ToolTip = "When a Tween reaches the end, it will restart from the end and go backward"),
    Restart UMETA(ToolTip = "When a Tween reaches the end, it will restart from the beginning")
};

UENUM()
enum class ETweenTargetType : uint8 {
    Actor,
    SceneComponent,
    UMG,
    Material,
    Custom
};

UENUM()
enum class ETweenVectorType : uint8 {
    MoveTo,
    MoveBy,
    ScaleTo,
    ScaleBy,
    Custom
};


UENUM()
enum class ETweenVector2DType : uint8 {
    MoveTo,
    MoveBy,
    ScaleTo,
    ScaleBy,
    ShearTo,
    Custom
};

UENUM()
enum class ETweenRotatorType : uint8 {
    RotateTo,
    RotateBy
};


UENUM()
enum class ETweenLinearColorType : uint8 {
    MaterialVectorFromTo,
    MaterialVectorTo,
};

UENUM()
enum class ETweenFloatType : uint8 {
    MaterialScalarFromTo,
    MaterialScalarTo,
    RotateAroundPoint,
    FollowSpline,
    WidgetAngleTo,
    WidgetOpacityTo,
    Custom
};


UENUM()
enum class ETweenReferenceAxis : uint8 {
    XAxis,
    YAxis,
    ZAxis
};

UENUM()
enum class ETweenGenericType : uint8 {
    Any UMETA(ToolTip = "Any type of Tween."),
    Move UMETA(ToolTip = "Tweens that move either an Actor, SceneComponent or a Widget."),
    Scale UMETA(ToolTip = "Tweens that scale either an Actor, SceneComponent or a Widget."),
    Rotate UMETA(ToolTip = "Tweens that rotate either an Actor or a SceneComponent."),
    RotateAroundPoint UMETA(ToolTip = "Tweens that rotate around a pivot point either an Actor or a SceneComponent."),
    FollowSpline UMETA(ToolTip = "Tweens that make either an Actor or a SceneComponent follow a spline."),
    MaterialVector UMETA(ToolTip = "Tweens that modify a Vector property of a material."),
    MaterialScalar UMETA(ToolTip = "Tweens that modify a Scalar property of a material."),
    WidgetAngle UMETA(ToolTip = "Tweens that rotate a Widget."),
    WidgetOpacity UMETA(ToolTip = "Tweens that modify the opacity value of a Widget."),
    WidgetShear UMETA(ToolTip = "Tweens that modify the shear value of a Widget."),
    CustomVector UMETA(ToolTip = "Custom Vector Tweens."),
    CustomFloat UMETA(ToolTip = "Custom Float Tweens."),
    CustomVector2D UMETA(ToolTip = "Custom Vector2D Tweens."),
};


