// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Asset/UIParticleAsset.h"
#include "Widgets/Images/SImage.h"

class FEasyParticleState;
class FEasyParticleRootState;

DECLARE_CYCLE_STAT(TEXT("UIParticle Tick"), STAT_UIParticleTickTime, STATGROUP_Slate);
DECLARE_CYCLE_STAT(TEXT("UIParticle OnPaint"), STAT_UIParticleOnPaintTime, STATGROUP_Slate);
DECLARE_CYCLE_STAT(TEXT("UIParticle SpawnParticle"), STAT_UIParticleSpawnParticle, STATGROUP_Slate);
DECLARE_DWORD_COUNTER_STAT(TEXT("UIParticle Count"), STAT_UIParticleCount, STATGROUP_Slate);

struct FEasyParticleChildEmitterArray
{
	UUIParticleEmitterAsset* ChildrenAsset;
	FDateTime EmitterStartTime;

	//TArray<TSharedPtr<FEasyParticleState, ESPMode::ThreadSafe>> ChildrenParticle;
	TArray<FEasyParticleState*> ChildrenParticle;
	TArray<int32> DeadParticleIndexPool;

	int32 ZOrderOffset;
	float ActivityInParentLifeTime;
	bool FollowParentPosition;
	float FollowParentSpeedPercent;

	/**********Update Every Frame***********/
	bool IsStartEmitter;
	float LastFractionalNum;
	int32 CountToSpawn;
	
	int32 GetActivitedParticleNum();
};

class FEasyParticleState
{
public:
	/***************CurState ,Update in Tick*************/
	float SpawnParticleDeltaTime;
	float CurLifetime;
	float CurPercent;
	float CurSize;
	float CurSizeX;
	float CurSizeY;
	float CurRotation;
	float CurAngle;
	FVector2D CurSpeed;
	FVector2D CurPos;
	FLinearColor CurColor;
	FVector2D CurPivot;
	float CurDelay;
	float RootSpanTime;
	float WholeScale = 1;
	float ScaleFollowSpeedDirection;
	float ScaleFollowSpeedVertical;
	FVector2D CurWidgetPosition;
	float CurWidgetRotation;
	bool IsFirstTick;
	bool IsNeedPaint;
	FPaintGeometry PaintGeometryCache;
	FLinearColor PaintColorCache;
	TArray<FEasyParticleChildEmitterArray> ChildrenParticleArray;

	/***************RandomKey ,Set in Create*************/
	float Deadtime;
	float ParticleEmitAngle;
	float RotationStart;
	float DirectionScale;
	float VerticalDirectionScale;
	EEmitterType EmitterType;
	EPositionType PositionType;
	EParticleDrawEffect DrawEffect;
	FVector2D StartSpeed;
	bool FollowParentPosition;
	float FollowParentSpeedPercent;

	//LerpKey//
	float SizeLerpKey;
	FLerpKeyVector2D SizeXLerpKey;
	FLerpKeyVector2D SizeYLerpKey;
	float RotationStartLerpKey;
	float RotationSpeedLerpKey;
	FLerpKeyColor ColorLerpKey;
	FLerpKeyVector2D PivotLerpKey;
	float PositionXLerpKey;
	float PositionYLerpKey;
	float ParticleLifeSpanLerpKey;
	FLerpKeyVector2D GravityLerpKey;
	float StartSpeedLerpKey;
	float AirResistanceLerpKey;
	float RadialAccelerationLerpKey;
	float TangentialAccelerationLerpKey;
	float ScaleFollowSpeedDirectionLerpKey;
	float ScaleFollowSpeedVerticalLerpKey;
	float VerticalDirectionScaleLerpKey;
	float DirectionScaleLerpKey;
	float SineDirectionStartLerpKey;
	float SineDirectionSpeedLerpKey;
	float SineDirectionRangePercent;
	float SpawnParticlePerSecondLerpKey;
	float MaxParticleCountLerpKey;
	float ParticleEmitAngleLerpKey;
	TArray<float> ScalarParameterLerpKeys;
	TArray<float> ScalarParameterWhenStartLerpKeys;
	float EmitPositionLerpKey;
	float EmitPosRangeLerpKey;

	//Gravity_Configuration//
	FVector2D Gravity;
	FVector2D SourcePos;
	float RadialAcceleration;
	float TangentialAcceleration;
	float SineDirectionStart;
	float SineDirectionSpeed;
	float AirResistance;
	float TotalDelay;
	int32 LayerOrder;
	int32 ZOrder;

	//Radial_Configuration//
	float Radius;
	float DegreePerSecond;
	float RadiusPercent;
	float DegreePerSecondPercent;

	/***************Particle Info ,Set in Create*************/
	FSlateBrush ImageBrush;
	UObject* ParticleResource;
	UUIParticleEmitterAsset* RootAsset;
	UUIParticleEmitterAsset* SelfAsset;
	bool IsRoot;
	bool bStopEmit;
	bool InPool;
	FVector2D StartPositionLocal;
	FVector2D StartWidgetPositionWorld;
	FEasyParticleState* ParentParticle;
	FEasyParticleRootState* RootParticle;

	/***************Functions*************/
	FEasyParticleState();
	virtual ~FEasyParticleState();

	void InitWithAsset(UUIParticleEmitterAsset* DataAsset);
	void CaculateSpawnParticleCount();
	void TrySpawnParticle(const FGeometry& AllottedGeometry);
	FEasyParticleState* CreateParticle(const FGeometry& AllottedGeometry, int32 ChildParticlePoolIndex);

	void UpdateParticle(float dt, const FGeometry& AllottedGeometry);
	void ResetStateProperty();
	void TickSelf(const float InDeltaTime, const FGeometry& AllottedGeometry, FVector2D InCurWidgetPosition, float InCurWidgetRotation, float RootScale = 1.0, float InRootSpanTime = 0);
	
	void CaculatePaintParams(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FWidgetStyle& InWidgetStyle);
	void OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled);
	void OnPaintSelf(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled);

	void SetInPool(bool inpool);
	bool IsInPool();

	void StopEmit();
	bool IsParticleEnd();
	bool IsEmitterEnd();
	bool IsAllChildrenEnd();
	int GetChildrenParticleCount();
	int GetChildrenParticleCountInPool();

};

class FEasyParticleRootState : public FEasyParticleState
{
public:
	FEasyParticleRootState();
	~FEasyParticleRootState();

	/***************CurState ,Update in Tick*************/
	bool FirstRootTick;

	/***************Functions*************/
	void FirstTick(const float FirstDeltaTime, const FGeometry& AllottedGeometry);
	void TickTick(const float FirstDeltaTime, const FGeometry& AllottedGeometry);
	void ResetRoot();
	void InitRoot(UUIParticleEmitterAsset* RootAsset, float ActiveDelay, int32 RootZOrder = 0);
};

class FEasyParticleGroupState
{
public:
	FEasyParticleGroupState();
	~FEasyParticleGroupState();

	TArray<TSharedPtr<FEasyParticleRootState, ESPMode::ThreadSafe>> Emitters;
	
	bool FirstRootTick;

public:
	bool IsEnd();
	void InitWithAsset(UUIParticleAsset* Asset);
	int GetParticleCount();
	int GetParticleCountInPool();
	bool Play();
	void Stop();
	void StopEmit();

	void Tick(const float InDeltaTime, const FGeometry& AllottedGeometry);
	void TrySpawnParticle(const FGeometry& AllottedGeometry);
	void OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled);
	void FirstTick(const float FirstDeltaTime, const FGeometry& AllottedGeometry);
};