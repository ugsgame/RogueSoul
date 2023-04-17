// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "Asset/UIParticleEmitterAsset.h"
#include "UIParticlePrivatePCH.h"
#include "Asset/UIParticleEmitterAsset.h"

UUIParticleEmitterAsset::UUIParticleEmitterAsset(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	AutoPlay = false;
	StartTimeOffset = 0;
	SamplingTimes = 2;

    EmitSeconds = 1;
	MaxParticleCount.Type = EUIParticlePropertyType::Float;
	MaxParticleCount.FloatValue = 50;

	SpawnParticlePerSecond.Type = EUIParticlePropertyType::Float;
	SpawnParticlePerSecond.FloatValue = 10;

	DirectionScale.Type = EUIParticlePropertyType::Float;
	DirectionScale.FloatValue = 1;

	VerticalDirectionScale.Type = EUIParticlePropertyType::Float;
	VerticalDirectionScale.FloatValue = 1;

	ParticleEmitAngle.Type = EUIParticlePropertyType::FloatRange;
	ParticleEmitAngle.FloatRangeValue.Min = 0;
	ParticleEmitAngle.FloatRangeValue.Max = 720;

	StartSpeed.Type = EUIParticlePropertyType::FloatRange;
	StartSpeed.FloatRangeValue.Min = 10;
	StartSpeed.FloatRangeValue.Max = 50;

	AutoEmitPosRange = false;
	AutoScale = false;
	ScaleByX = true;
	RotationFollowSpeed = false;
	UseScaleFollowSpeedVertical = false;
	UseScaleFollowSpeedDirection = false;
	UseSeparateSize = false;
	DesignSize = FVector2D(200, 200);


	Pivot.Type = EUIParticlePropertyType::Vector2D;
	Pivot.Vector2DValue = FVector2D(0.5, 0.5);

	Color.Type = EUIParticlePropertyType::LinearColorCurveRange;

	LifeSpan.Type = EUIParticlePropertyType::Float;
	LifeSpan.FloatValue = 1;
	LifeSpan.FloatRangeValue.Min = 1;
	LifeSpan.FloatRangeValue.Max = 1.5;

	Gravity.Type = EUIParticlePropertyType::Vector2D;
	Gravity.Vector2DValue = FVector2D(0, 0);

	Size.Type = EUIParticlePropertyType::FloatRange;
	Size.FloatRangeValue.Min = 20;
	Size.FloatRangeValue.Max = 50;

	Radius.Type = EUIParticlePropertyType::FloatRange;
	Radius.FloatRangeValue.Min = 20;
	Radius.FloatRangeValue.Max = 50;
	
	DegreePerSecond.Type = EUIParticlePropertyType::FloatRange;
	DegreePerSecond.FloatRangeValue.Min = 20;
	DegreePerSecond.FloatRangeValue.Max = 50;

	ScaleFollowSpeedDirection.Type = EUIParticlePropertyType::Float;
	ScaleFollowSpeedDirection.FloatValue = 1;

	ScaleFollowSpeedVertical.Type = EUIParticlePropertyType::Float;
	ScaleFollowSpeedVertical.FloatValue = 1;

	SineDirectionStart.Type = EUIParticlePropertyType::Float;
	SineDirectionStart.FloatValue = 0;
	SineDirectionSpeed.Type = EUIParticlePropertyType::Float;
	SineDirectionSpeed.FloatValue = 0;
	SineDirectionRange.Type = EUIParticlePropertyType::Float;
	SineDirectionRange.FloatValue = 0;

	RadialAcceleration.Type = EUIParticlePropertyType::Float;
	RadialAcceleration.FloatValue = 0;
	TangentialAcceleration.Type = EUIParticlePropertyType::Float;
	TangentialAcceleration.FloatValue = 0;

	AirResistance.Type = EUIParticlePropertyType::Float;
	AirResistance.FloatValue = 0;

	EmitPosition.X.SetCurveType(ECurveType::EmitLifeTime);
	EmitPosition.Y.SetCurveType(ECurveType::EmitLifeTime);
	EmitPosition.X.SetCurveLoop(true);
	EmitPosition.Y.SetCurveLoop(true);
	EmitPosition.X.FloatValue = 0;
	EmitPosition.Y.FloatValue = 0;

	SpawnParticlePerSecond.SetCurveType(ECurveType::EmitLifeTime);
	MaxParticleCount.SetCurveType(ECurveType::EmitLifeTime);

	RotationStart.SetCurveType(ECurveType::EmitLifeTime);
	SineDirectionStart.SetCurveType(ECurveType::EmitLifeTime);
	ParticleEmitAngle.SetCurveType(ECurveType::EmitLifeTime);
	StartSpeed.SetCurveType(ECurveType::EmitLifeTime);

	EmitAngleByWidgetAngle = false;
}
