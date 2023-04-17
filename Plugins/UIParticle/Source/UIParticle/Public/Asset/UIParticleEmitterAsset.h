// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Curves/CurveVector.h"
#include "Curves/CurveLinearColor.h"
#include "Curves/CurveFloat.h"
#include "UIParticleEmitterAsset.generated.h"

class UUIParticleEmitterAsset;

UENUM()
enum class  ECurveType : uint8
{
	ParticleLifePercent = 0,
	ParticleLifeTime = 1,
	EmitLifeTime = 2,
};

USTRUCT()
struct FLerpKeyVector2D
{
	GENERATED_BODY()
	float X, Y;
	void SetValue(float InX, float InY)
	{
		X = InX;
		Y = InY;
	};

	void Random()
	{
		X = FMath::FRandRange(0, 1);
		Y = FMath::FRandRange(0, 1);
	};
};

USTRUCT()
struct FLerpKeyColor
{
	GENERATED_BODY()
	float R,G,B,A;
	void SetValue(float InR, float InG, float InB, float InA)
	{
		R = InR;
		G = InG;
		B = InB;
		A = InA;
	};
	void Random()
	{
		R = FMath::FRandRange(0, 1);
		G = FMath::FRandRange(0, 1);
		B = FMath::FRandRange(0, 1);
		A = FMath::FRandRange(0, 1);
	};
};


USTRUCT()
struct FUIParticleFloatCurve
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = Particle)
		FRichCurve CurveData;

	UPROPERTY(EditAnywhere, Category = Particle)
		ECurveType CurveType;

	UPROPERTY(EditAnywhere, Category = Particle)
		bool Loop;

	void Reset()
	{
		Loop = false;
		CurveType = ECurveType::EmitLifeTime;
		CurveData.Reset();
		CurveData.SetDefaultValue(0);
	};

	float GetValue(float InParticleLifePercent , float InParticleTime , float EmitTime) const
	{
		float Ret = 0;
		switch (CurveType)
		{
			case ECurveType::ParticleLifePercent:
			{
				Ret = CurveData.Eval(InParticleLifePercent, 0);
				break;
			}
			case ECurveType::ParticleLifeTime:
			{
				if (Loop)
				{
					float MinTime;
					float MaxTime;
					CurveData.GetTimeRange(MinTime, MaxTime);
					float curvetime = 0;
					if (MaxTime > 0)
					{
						curvetime = FMath::Fmod(InParticleTime, MaxTime);
					}
					Ret = CurveData.Eval(curvetime, 0);
				}
				else
				{
					Ret = CurveData.Eval(InParticleTime, 0);
				}
				break;
			}
			case ECurveType::EmitLifeTime:
			{		
				if (Loop)
				{
					float MinTime;
					float MaxTime;
					CurveData.GetTimeRange(MinTime, MaxTime);
					float curvetime = 0;
					if (MaxTime > 0)
					{
						curvetime = FMath::Fmod(EmitTime, MaxTime);
					}
					Ret = CurveData.Eval(curvetime, 0);
				}
				else
				{
					Ret = CurveData.Eval(EmitTime, 0);
				}
				break;
			}
		}
		return Ret;
	}
};

USTRUCT(BlueprintType)
struct FChildEmitter
{
	GENERATED_BODY()
	UPROPERTY( EditAnywhere , BlueprintReadWrite , Category = Children , meta = ( UIMin = "0" , UIMax = "1" ))
		float ActivityInParentLifeTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ChildrenEmitters)
		bool FollowParentPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ChildrenEmitters)
		float FollowParentSpeedPercent;
	// 0 is same with parent particle.-1 is under parent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ChildrenEmitters)
		int32 ZOrderOffset;
	UPROPERTY( EditAnywhere , BlueprintReadWrite , Category = Children)
		UUIParticleEmitterAsset* ChildrenAsset;
};

UENUM(BlueprintType)
enum class  EEmitterType : uint8
{
	Gravity = 0,
	Radial = 1,
	Curve = 2,
};

UENUM(BlueprintType)
enum class  EPositionType : uint8
{
	FREE = 0,
	RELATIVE = 1,
};

UENUM(BlueprintType)
enum class EParticleDrawEffect : uint8
{
    /** No effect applied */
    None                    = 0,
    /** Advanced: Draw the element with no blending */
    NoBlending            = 1 << 0,
    /** Advanced: Blend using pre-multiplied alpha. Ignored if NoBlending is set. */
    PreMultipliedAlpha    = 1 << 1,
    /** Advanced: No gamma correction should be done */
    NoGamma                = 1 << 2,
    /** Advanced: Change the alpha value to 1 - Alpha. */
    InvertAlpha            = 1 << 3,
    
    // ^^ These Match ESlateBatchDrawFlag ^^
    
    /** Disables pixel snapping */
    NoPixelSnapping        = 1 << 4,
    /** Draw the element with a disabled effect */
    DisabledEffect        = 1 << 5,
    /** Advanced: Don't read from texture alpha channel */
    IgnoreTextureAlpha    = 1 << 6,
    
    /** Advanced: Existing Gamma correction should be reversed */
    ReverseGamma            = 1 << 7
};

USTRUCT(BlueprintType)
struct FRange_FloatCurve
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleFloatCurve Min;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleFloatCurve Max;

	float GetValue(float InParticleLifePercent, float InParticleTime, float InEmitTime, float LerpKey)
	{
		float MinValue = Min.GetValue(InParticleLifePercent,InParticleTime, InEmitTime);
		float MaxValue = Max.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
		return FMath::Lerp(MinValue, MaxValue, LerpKey);
	};

	void Reset()
	{
		Min.Reset();
		Max.Reset();
	};

};

USTRUCT()
struct FUIParticleLinearColorCurve
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, Category = Particle)
		FRichCurve ColorCurves[4];

	UPROPERTY(EditAnywhere, Category = Particle)
		ECurveType CurveType;

	UPROPERTY(EditAnywhere, Category = Particle)
		bool Loop;



	FLinearColor GetValueInner(float InTime) const
	{
		return FLinearColor(ColorCurves[0].Eval(InTime, 1), ColorCurves[1].Eval(InTime, 1), ColorCurves[2].Eval(InTime, 1), ColorCurves[3].Eval(InTime, 1));
	}

	FLinearColor GetValue(float InParticleLifePercent, float InParticleTime, float InEmitTime) const
	{
		FLinearColor Ret;
		switch (CurveType)
		{
			case ECurveType::ParticleLifePercent:
			{
				Ret = GetValueInner(InParticleLifePercent);
				break;
			}
			case ECurveType::ParticleLifeTime:
			{
				if (Loop)
				{
					float MinTime=0;
					float MaxTime=0;

					for (int32 i = 0; i < 4; i++)
					{
						float Min,Max;
						ColorCurves[i].GetTimeRange(Min, Max);
						if (Max > MaxTime)
						{
							MaxTime = Max;
						}
					}
					float curvetime = 0;
					if (MaxTime > 0)
					{
						curvetime = FMath::Fmod(InParticleTime, MaxTime);
					}
					Ret = GetValueInner(curvetime);
				}
				else
				{
					Ret = GetValueInner(InParticleTime);
				}
				break;
			}
			case ECurveType::EmitLifeTime:
			{
				if (Loop)
				{
					float MinTime = 0;
					float MaxTime = 0;

					for (int32 i = 0; i < 4; i++)
					{
						float Min, Max;
						ColorCurves[i].GetTimeRange(Min, Max);
						if (Max > MaxTime)
						{
							MaxTime = Max;
						}
					}
					float curvetime = 0;
					if (MaxTime > 0)
					{
						curvetime = FMath::Fmod(InEmitTime, MaxTime);
					}
					Ret = GetValueInner(curvetime);
				}
				else
				{
					Ret = GetValueInner(InEmitTime);
				}
				break;
			}
		}
		return Ret;
	};

	void Reset()
	{
		for (int32 i = 0; i < 4; i++)
		{
			ColorCurves[i].Reset();
		}
		CurveType = ECurveType::EmitLifeTime;
		Loop = false;
	};

};

USTRUCT(BlueprintType)
struct FRange_LinearColorCurve
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleLinearColorCurve Min;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleLinearColorCurve Max;

	UPROPERTY(EditAnywhere, Category = Particle, AdvancedDisplay, meta = (DisplayName = "RandomKey G=R"))
		bool RandomKey_R_G;
	UPROPERTY(EditAnywhere, Category = Particle, AdvancedDisplay, meta = (DisplayName = "RandomKey B=R"))
		bool RandomKey_R_B;
	UPROPERTY(EditAnywhere, Category = Particle, AdvancedDisplay, meta = (DisplayName = "RandomKey A=R"))
		bool RandomKey_R_A;
	UPROPERTY(EditAnywhere, Category = Particle, AdvancedDisplay, meta = (DisplayName = "RandomKey B=G"))
		bool RandomKey_G_B;
	UPROPERTY(EditAnywhere, Category = Particle, AdvancedDisplay, meta = (DisplayName = "RandomKey A=G"))
		bool RandomKey_G_A;
	UPROPERTY(EditAnywhere, Category = Particle, AdvancedDisplay, meta = (DisplayName = "RandomKey A=B"))
		bool RandomKey_B_A;


	FLinearColor GetValue(float InParticleLifePercent, float InParticleTime, float InEmitTime, float LerpKey)
	{
		FLinearColor MinValue = Min.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
		FLinearColor MaxValue = Max.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
		return FMath::Lerp(MinValue, MaxValue, LerpKey);
	};

	FLinearColor GetValue(float InParticleLifePercent, float InParticleTime, float InEmitTime, FLerpKeyColor LerpKey)
	{
		float lerp_r = LerpKey.R;
		float lerp_g = LerpKey.G;
		float lerp_b = LerpKey.B;
		float lerp_a = LerpKey.A;
		if (RandomKey_R_G)
			lerp_g = lerp_r;
		if (RandomKey_R_B)
			lerp_b = lerp_r;
		if (RandomKey_R_A)
			lerp_a = lerp_r;
		if (RandomKey_G_B)
			lerp_b = lerp_g;
		if (RandomKey_G_A)
			lerp_a = lerp_g;
		if (RandomKey_B_A)
			lerp_a = lerp_b;

		FLinearColor MinValue = Min.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
		FLinearColor MaxValue = Max.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
		return FLinearColor(FMath::Lerp(MinValue.R, MaxValue.R, lerp_r), FMath::Lerp(MinValue.G, MaxValue.G, lerp_g), FMath::Lerp(MinValue.B, MaxValue.B, lerp_b), FMath::Lerp(MinValue.A, MaxValue.A, lerp_a));
	};

	void Reset()
	{
		Min.Reset();
		Max.Reset();
		RandomKey_R_G = false;
		RandomKey_R_B = false;
		RandomKey_R_A = false;
		RandomKey_G_B = false;
		RandomKey_G_A = false;
		RandomKey_B_A = false;
	};
};

USTRUCT(BlueprintType)
struct FRange_Float
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = Particle, BlueprintReadWrite )
		float Min;
	UPROPERTY(EditAnywhere, Category = Particle, BlueprintReadWrite )
		float Max;


	float GetValue()
	{
		return FMath::FRandRange(Min, Max);
	};
	float GetValue(float LerpKey)
	{
		return FMath::Lerp(Min, Max, LerpKey);
	};

};

USTRUCT(BlueprintType)
struct FScalarParamFloat
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle )
		FName ScalarParamName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle )
		FRange_Float Value;

	float GetValue()
	{
		return Value.GetValue();
	};
};

USTRUCT(BlueprintType)
struct FRange_Vector2D
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, Category = Particle, BlueprintReadWrite)
		FVector2D Min;
	UPROPERTY(EditAnywhere, Category = Particle, BlueprintReadWrite)
		FVector2D Max;

	UPROPERTY(EditAnywhere, Category = Particle, AdvancedDisplay, meta = ( DisplayName = "RandomKey X=Y"))
		bool RandomKey_X_Y;

	FVector2D GetValue()
	{
		return FVector2D(FMath::FRandRange(Min.X, Max.X), FMath::FRandRange(Min.Y, Max.Y));
	};
	FVector2D GetValue(float LerpKey)
	{
		return FMath::Lerp(Min,Max,LerpKey);
	};
	FVector2D GetValue(FLerpKeyVector2D LerpKey)
	{
		float lerp_x = LerpKey.X;
		float lerp_y = LerpKey.Y;
		if (RandomKey_X_Y)
			lerp_y = lerp_x;
		return FVector2D(FMath::Lerp(Min.X, Max.X, lerp_x), FMath::Lerp(Min.Y, Max.Y, lerp_y));
	};

	void Reset()
	{
		Min = FVector2D(0, 0);
		Max = FVector2D(0, 0);
		RandomKey_X_Y = false;
	};
};

/******test*******/
UENUM(BlueprintType)
enum class EUIParticlePropertyType : uint8
{
	Float = 0,
	FloatRange,
	FloatCurve,
	FloatCurveRange,
	Vector2D,
	Vector2DRange,
	LinearColorCurve,
	LinearColorCurveRange,
};


USTRUCT()
struct FUIParticleProperty
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, Category = UIParticleProperty)
		EUIParticlePropertyType Type;

	UPROPERTY(EditAnywhere, Category = UIParticleProperty, meta = (EnumCondition = 0))
		float FloatValue;
	UPROPERTY(EditAnywhere, Category = UIParticleProperty, meta = (EnumCondition = 1))
		FRange_Float FloatRangeValue;
	UPROPERTY(EditAnywhere, Category = UIParticleProperty, meta = (EnumCondition = 2))
		FUIParticleFloatCurve FloatCurveValue;
	UPROPERTY(EditAnywhere, Category = UIParticleProperty, meta = (EnumCondition = 3))
		FRange_FloatCurve FloatCurveRangeValue;
	UPROPERTY(EditAnywhere, Category = UIParticleProperty, meta = (EnumCondition = 4))
		FVector2D Vector2DValue;
	UPROPERTY(EditAnywhere, Category = UIParticleProperty, meta = (EnumCondition = 5))
		FRange_Vector2D Vector2DRangeValue;
	UPROPERTY(EditAnywhere, Category = UIParticleProperty, meta = (EnumCondition = 6))
		FUIParticleLinearColorCurve LinearColorCurveValue;
	UPROPERTY(EditAnywhere, Category = UIParticleProperty, meta = (EnumCondition = 7))
		FRange_LinearColorCurve LinearColorCurveRangeValue;
	

	void SetCurveType(ECurveType InCurveType)
	{
		FloatCurveValue.CurveType = InCurveType;
		FloatCurveRangeValue.Min.CurveType = InCurveType;
		FloatCurveRangeValue.Max.CurveType = InCurveType;
	}
	void SetCurveLoop(bool InLoop)
	{
		FloatCurveValue.Loop = InLoop;
		FloatCurveRangeValue.Min.Loop = InLoop;
		FloatCurveRangeValue.Max.Loop = InLoop;
	}

	FUIParticleProperty()
	{
		FloatValue = 1;
		FloatRangeValue.Min = 20;
		FloatRangeValue.Max = 50;

		FloatCurveValue.CurveData.SetDefaultValue(10);
		FloatCurveValue.CurveData.AddKey(0, 10);
		FloatCurveValue.CurveData.AddKey(1, 30);

		FloatCurveRangeValue.Min.CurveData.SetDefaultValue(10);
		FloatCurveRangeValue.Min.CurveData.AddKey(0, 10);
		FloatCurveRangeValue.Min.CurveData.AddKey(1, 30);

		FloatCurveRangeValue.Max.CurveData.SetDefaultValue(20);
		FloatCurveRangeValue.Max.CurveData.AddKey(0, 20);
		FloatCurveRangeValue.Max.CurveData.AddKey(1, 50);

		Vector2DValue = FVector2D(10, 10);

		Vector2DRangeValue.Min = FVector2D(10, 10);
		Vector2DRangeValue.Max = FVector2D(20, 20);

		LinearColorCurveRangeValue.Min.ColorCurves[0].AddKey(0, 1);
		LinearColorCurveRangeValue.Min.ColorCurves[1].AddKey(0, 1);
		LinearColorCurveRangeValue.Min.ColorCurves[2].AddKey(0, 1);

		LinearColorCurveRangeValue.Min.ColorCurves[3].AddKey(0, 0);
		LinearColorCurveRangeValue.Min.ColorCurves[3].AddKey(0.5, 1);
		LinearColorCurveRangeValue.Min.ColorCurves[3].AddKey(1, 0);

		LinearColorCurveRangeValue.Max.ColorCurves[0].AddKey(0, 1);
		LinearColorCurveRangeValue.Max.ColorCurves[1].AddKey(0, 1);
		LinearColorCurveRangeValue.Max.ColorCurves[2].AddKey(0, 1);

		LinearColorCurveRangeValue.Max.ColorCurves[3].AddKey(0, 0);
		LinearColorCurveRangeValue.Max.ColorCurves[3].AddKey(0.5, 1);
		LinearColorCurveRangeValue.Max.ColorCurves[3].AddKey(1, 0);

		LinearColorCurveValue.ColorCurves[0].AddKey(0, 1);
		LinearColorCurveValue.ColorCurves[1].AddKey(0, 1);
		LinearColorCurveValue.ColorCurves[2].AddKey(0, 1);

		LinearColorCurveValue.ColorCurves[3].AddKey(0, 0);
		LinearColorCurveValue.ColorCurves[3].AddKey(0.5, 1);
		LinearColorCurveValue.ColorCurves[3].AddKey(1, 0);

		for (int32 i = 0; i < 4; i++)
		{
			LinearColorCurveRangeValue.Min.ColorCurves[i].SetDefaultValue(1);
			LinearColorCurveRangeValue.Max.ColorCurves[i].SetDefaultValue(1);
			LinearColorCurveValue.ColorCurves[i].SetDefaultValue(1);
		}

	};

	void Reset()
	{
		Type = EUIParticlePropertyType::Float;
		FloatValue = 0;
		FloatRangeValue.Min = 0;
		FloatRangeValue.Max = 0;
		FloatCurveValue.Reset();
		FloatCurveRangeValue.Reset();
		Vector2DValue = FVector2D(0, 0);
		Vector2DRangeValue.Reset();
		LinearColorCurveValue.Reset();
		LinearColorCurveRangeValue.Reset();
	}

	FLinearColor GetValue_Color(float InParticleLifePercent, float InParticleTime, float InEmitTime, FLerpKeyColor LerpKey)
	{
		FLinearColor Ret;
		switch (Type)
		{
			case EUIParticlePropertyType::Float:
			{
				Ret = FLinearColor(FloatValue, FloatValue, FloatValue, FloatValue);
			}
				break;
			case EUIParticlePropertyType::FloatRange:
			{
				float Value = FloatRangeValue.GetValue(LerpKey.R);
				Ret = FLinearColor(Value, Value, Value, Value);
			}
				break;
			case EUIParticlePropertyType::FloatCurve:
			{
				float Value = FloatCurveValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
				Ret = FLinearColor(Value, Value, Value, Value);
			}
				break;
			case EUIParticlePropertyType::FloatCurveRange:
			{
				float Value = FloatCurveRangeValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime ,LerpKey.R);
				Ret = FLinearColor(Value, Value, Value, Value);
			}
				break;
			case EUIParticlePropertyType::Vector2D:
			{
				float Value = Vector2DValue.X;
				Ret = FLinearColor(Value, Value, Value, Value);
			}
				break;
			case EUIParticlePropertyType::Vector2DRange:
			{
				FLerpKeyVector2D Temp;
				Temp.SetValue(LerpKey.R, LerpKey.G);
				FVector2D Value = Vector2DRangeValue.GetValue(Temp);
				Ret = FLinearColor(Value.X, Value.Y, 0, 0);
			}
				break;
			case EUIParticlePropertyType::LinearColorCurve:
			{
				Ret = LinearColorCurveValue.GetValue( InParticleLifePercent, InParticleTime, InEmitTime);
			}
				break;
			case EUIParticlePropertyType::LinearColorCurveRange:
			{
				Ret = LinearColorCurveRangeValue.GetValue( InParticleLifePercent, InParticleTime, InEmitTime, LerpKey);
			}
				break;
		};
		return Ret;
	};
	float GetValue_Float(float InParticleLifePercent, float InParticleTime, float InEmitTime, float LerpKey)
	{
		float Ret = 0;
		switch (Type)
		{
			case EUIParticlePropertyType::Float:
			{
				Ret = FloatValue;
			}
				break;
			case EUIParticlePropertyType::FloatRange:
			{
				Ret = FloatRangeValue.GetValue(LerpKey);
			}
				break;
			case EUIParticlePropertyType::FloatCurve:
			{
				Ret = FloatCurveValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
			}
				break;
			case EUIParticlePropertyType::FloatCurveRange:
			{
				Ret = FloatCurveRangeValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime, LerpKey);
			}
				break;
			case EUIParticlePropertyType::Vector2D:
			{
				Ret = Vector2DValue.X;
			}
				break;
			case EUIParticlePropertyType::Vector2DRange:
			{
				Ret = Vector2DRangeValue.GetValue(LerpKey).X;
			}
				break;
			case EUIParticlePropertyType::LinearColorCurve:
			{
				Ret = LinearColorCurveValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime).R;
			}
				break;
			case EUIParticlePropertyType::LinearColorCurveRange:
			{
				Ret = LinearColorCurveRangeValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime, LerpKey).R;
			}
				break;
		};
		return Ret;
	};

	FVector2D GetValue_Vector2D(float InParticleLifePercent, float InParticleTime, float InEmitTime, FLerpKeyVector2D LerpKey)
	{
		FVector2D Ret;
		switch (Type)
		{
			case EUIParticlePropertyType::Float:
			{
				float Value = FloatValue;
				Ret = FVector2D(Value, Value);
			}
				break;
			case EUIParticlePropertyType::FloatRange:
			{
				float Value = FloatRangeValue.GetValue(LerpKey.X);
				Ret = FVector2D(Value, Value);
			}
				break;
			case EUIParticlePropertyType::FloatCurve:
			{
				float Value = FloatCurveValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
				Ret = FVector2D(Value, Value);
			}
				break;
			case EUIParticlePropertyType::FloatCurveRange:
			{
				float Value = FloatCurveRangeValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime, LerpKey.X);
				Ret = FVector2D(Value, Value);
			}
				break;
			case EUIParticlePropertyType::Vector2D:
			{
				Ret = Vector2DValue;
			}
				break;
			case EUIParticlePropertyType::Vector2DRange:
			{
				Ret = Vector2DRangeValue.GetValue(LerpKey);
			}
				break;
			case EUIParticlePropertyType::LinearColorCurve:
			{
				FLinearColor Value = LinearColorCurveValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime);
				Ret = FVector2D(Value.R, Value.G);
			}
				break;
			case EUIParticlePropertyType::LinearColorCurveRange:
			{
				FLerpKeyColor Temp;
				Temp.SetValue(LerpKey.X, LerpKey.Y, 0, 0);
				FLinearColor Value = LinearColorCurveRangeValue.GetValue(InParticleLifePercent, InParticleTime, InEmitTime, Temp);
				Ret = FVector2D(Value.R, Value.G);
			}
				break;
		};
		return Ret;
	};
};


USTRUCT(BlueprintType)
struct FPosotion_Vector2DCurve
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty X;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty Y;

	FVector2D GetValue(float InParticleLifePercent, float InParticleTime, float InEmitTime, float LerpKey)
	{
		float CurX = X.GetValue_Float(InParticleLifePercent, InParticleTime, InEmitTime, LerpKey);
		float CurY = Y.GetValue_Float(InParticleLifePercent, InParticleTime, InEmitTime, LerpKey);
		return FVector2D(CurX, CurY);
	};

	void Reset()
	{
		X.Reset();
		Y.Reset();
	};
};

USTRUCT(BlueprintType)
struct FSize_Vector2DCurve
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty Min;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty Max;

	FVector2D GetValue(float InParticleLifePercent, float InParticleTime, float InEmitTime, float LerpKey)
	{
		float CurX = Min.GetValue_Float(InParticleLifePercent, InParticleTime, InEmitTime, LerpKey);
		float CurY = Max.GetValue_Float(InParticleLifePercent, InParticleTime, InEmitTime, LerpKey);
		return FVector2D(CurX, CurY);
	};
};

USTRUCT(BlueprintType)
struct FScalarParamCurve
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Particle)
		FName ScalarParamName;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty Value;

	float GetValue(float InParticleLifePercent, float InParticleTime, float EmitTime, float LerpKey)
	{
		float Ret = Value.GetValue_Float(InParticleLifePercent, InParticleTime, EmitTime, LerpKey);
		return Ret;
	};
};

UCLASS(BlueprintType)
class UIPARTICLE_API  UUIParticleEmitterAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//Root Setting//
	//Auto Play//
	UPROPERTY(EditAnywhere, Category = Root)
		bool AutoPlay;
	//Very expensive.Start playing at a specific time. Tick {SamplingTimes} when start.The smaller, the more accurate // 
	UPROPERTY(EditAnywhere, Category = Root, meta = (ClampMin = 0, ClampMax = 100))
		float StartTimeOffset;
	//Very expensive.Sampling times from 0 to start playing .The bigger, the more accurate //
	UPROPERTY(EditAnywhere, Category = Root, meta = (UIMin = "2", UIMax = "20"))
		int32 SamplingTimes;

    //Emitter Setting//
    UPROPERTY(EditAnywhere , Category = Emitter )
        EEmitterType EmitterType;

    UPROPERTY(EditAnywhere , Category = Emitter , meta = ( UIMin = "-1" , UIMax = "10" ))
        float EmitSeconds;
	UPROPERTY(EditAnywhere, Category = Emitter)
		FUIParticleProperty MaxParticleCount;
	UPROPERTY(EditAnywhere, Category = Emitter )
		FUIParticleProperty SpawnParticlePerSecond;
	UPROPERTY(EditAnywhere, Category = Emitter)
		FUIParticleProperty ParticleEmitAngle;
	UPROPERTY(EditAnywhere, Category = Emitter)
		FRange_Vector2D EmitPosRange;
	UPROPERTY(EditAnywhere, Category = Emitter)
		FPosotion_Vector2DCurve EmitPosition;

	//Size to UMG
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Emitter)
		bool AutoEmitPosRange;
	//Auto Size to rendertransform scale x
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Emitter)
		bool AutoScale;
	//True is scale by x,false is scale by y
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Emitter, meta = (EditCondition = "AutoScale"))
		bool ScaleByX;
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Emitter, meta = (EditCondition = "AutoScale"))
		FVector2D DesignSize;
	//Add WidgetAngle To EmitAngle(Effectiveness only for root node)
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Emitter)
		bool EmitAngleByWidgetAngle;

    //Particle Setting//
	UPROPERTY(EditAnywhere, Category = Particle)
		EPositionType PositionType;

	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty LifeSpan;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty Size;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty Pivot;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty RotationStart;
	UPROPERTY(EditAnywhere, Category = Particle, meta = (EditCondition = "!RotationFollowSpeed"))
		FUIParticleProperty RotationSpeed;
	UPROPERTY(EditAnywhere, Category = Particle)
		FUIParticleProperty Color;
	UPROPERTY(EditAnywhere, Category = Particle, meta = (DisplayThumbnail = "true", DisplayName = "Image", AllowedClasses = "Texture,MaterialInterface,SlateTextureAtlasInterface"))
		UObject* ResourceObject;
	//Force rotation to speed direction
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Particle)
		bool RotationFollowSpeed;
	//Separate Size X and Y Size Must Range Type
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Particle)
		bool UseSeparateSize;
    
	//Gravity_Configuration//
	UPROPERTY(EditAnywhere, Category = Gravity_Configuration)
		FUIParticleProperty Gravity;
	UPROPERTY(EditAnywhere, Category = Gravity_Configuration)
		FUIParticleProperty StartSpeed;
	UPROPERTY(EditAnywhere, Category = Gravity_Configuration)
		FUIParticleProperty AirResistance;
	UPROPERTY(EditAnywhere, Category = Gravity_Configuration)
		FUIParticleProperty RadialAcceleration;
	UPROPERTY(EditAnywhere, Category = Gravity_Configuration)
		FUIParticleProperty TangentialAcceleration;

	//Radial_Configuration
	UPROPERTY(EditAnywhere, Category = Radial_Configuration)
		FUIParticleProperty Radius;
	UPROPERTY(EditAnywhere, Category = Radial_Configuration)
		FUIParticleProperty DegreePerSecond;
	
	//Curve_Configuration//
	UPROPERTY(EditAnywhere, Category = Curve_Configuration)
		FUIParticleProperty PositionX;
	UPROPERTY(EditAnywhere, Category = Curve_Configuration)
		FUIParticleProperty PositionY;

    UPROPERTY(EditAnywhere , Category = ChildrenEmitters)
        TArray<FChildEmitter> ChildrenEmitters;


	//Very expensive. Change material scalarparam every tick.
	UPROPERTY(EditAnywhere, Category = Experimentation)
		TArray<FScalarParamCurve> ScalarParams;
	//Very expensive. Change material scalarparam when start,set scalarparem "LifeSpan" when start
	UPROPERTY(EditAnywhere, Category = Experimentation)
		TArray<FScalarParamCurve> ScalarParamsWhenStart;

    UPROPERTY(EditAnywhere, Category = Experimentation)
        EParticleDrawEffect DrawEffect;
	//Scale By Speed Base Speed is 100, if Speed is 150 ,scaleX is 150/100*ScaleX. Usually use with RotationFollowSpeed
	UPROPERTY(EditAnywhere, Category = Experimentation)
		bool UseScaleFollowSpeedDirection;
	UPROPERTY(EditAnywhere, Category = Experimentation, meta = (EditCondition = "UseScaleFollowSpeedDirection"))
		FUIParticleProperty ScaleFollowSpeedDirection;
	//Scale By Speed Base Speed is 100, if Speed is 150 ,scaleX is 150/100*ScaleY. Usually use with RotationFollowSpeed
	UPROPERTY(EditAnywhere, Category = Experimentation)
		bool UseScaleFollowSpeedVertical;
	UPROPERTY(EditAnywhere, Category = Experimentation, meta = (EditCondition = "UseScaleFollowSpeedVertical"))
		FUIParticleProperty ScaleFollowSpeedVertical;

	//Usually use with RotationFollowSpeed
	UPROPERTY(EditAnywhere, Category = Experimentation)
		FUIParticleProperty DirectionScale;
	//Usually use with RotationFollowSpeed
	UPROPERTY(EditAnywhere, Category = Experimentation)
		FUIParticleProperty VerticalDirectionScale;


	UPROPERTY(EditAnywhere, Category = Experimentation)
		FUIParticleProperty SineDirectionStart;
	UPROPERTY(EditAnywhere, Category = Experimentation)
		FUIParticleProperty SineDirectionSpeed;
	UPROPERTY(EditAnywhere, Category = Experimentation)
		FUIParticleProperty SineDirectionRange;
	

	//Level Of Detail.SetLOD() function and GetLOD function;
	UPROPERTY(EditAnywhere, Category = Experimentation)
		TArray<UUIParticleEmitterAsset*> LevelOfDetail;

	void Reset()
	{
		AutoPlay = false;
		StartTimeOffset = 0;
		SamplingTimes = 2;
		EmitterType = EEmitterType::Gravity;
		EmitSeconds = 0;
		MaxParticleCount.Reset();
		SpawnParticlePerSecond.Reset();
		ParticleEmitAngle.Reset();
		EmitPosRange.Reset();
		EmitPosition.Reset();
		AutoEmitPosRange = false;
		AutoScale = false;
		ScaleByX = false;
		DesignSize = FVector2D(0, 0);

		//Particle Setting//
		PositionType = EPositionType::FREE;
		LifeSpan.Reset();
		Size.Reset();
		Pivot.Reset();
		RotationStart.Reset();
		RotationSpeed.Reset();
		Color.Reset();
		ResourceObject = nullptr;
		RotationFollowSpeed = false;
		UseSeparateSize = false;

		//Gravity_Configuration//
		Gravity.Reset();
		StartSpeed.Reset();
		AirResistance.Reset();
		RadialAcceleration.Reset();
		TangentialAcceleration.Reset();
		
		//Radial_Configuration
		Radius.Reset();
		DegreePerSecond.Reset();

		//Curve_Configuration//
		PositionX.Reset();
		PositionY.Reset();

		ChildrenEmitters.Empty();

		ScalarParams.Empty();
		ScalarParamsWhenStart.Empty();
		DrawEffect = EParticleDrawEffect::None;

		UseScaleFollowSpeedDirection = false;
		ScaleFollowSpeedDirection.Reset();

		UseScaleFollowSpeedVertical = false;
		ScaleFollowSpeedVertical.Reset();

		DirectionScale.Reset();
		VerticalDirectionScale.Reset();

		SineDirectionStart.Reset();
		SineDirectionSpeed.Reset();
		SineDirectionRange.Reset();

		EmitAngleByWidgetAngle = false;

		LevelOfDetail.Empty();
	};

	UUIParticleEmitterAsset* GetLODAsset(int32 levelindex = -1)
	{
		if (LevelOfDetail.Num() > 0)
		{
			if (levelindex >= 0 && levelindex < LevelOfDetail.Num())
			{
				if (LevelOfDetail[levelindex] != nullptr)
				{
					return LevelOfDetail[levelindex];
				}
				else
				{
					for (int32 i = levelindex; i >= 0; i--)
					{
						if (LevelOfDetail[i] != nullptr)
						{
							return LevelOfDetail[i];
						}
					}
				}
			}
		}
		return this;
	}
};
