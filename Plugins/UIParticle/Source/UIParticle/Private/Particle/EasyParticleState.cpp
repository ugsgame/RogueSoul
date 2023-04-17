// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "Particle/EasyParticleState.h"
#include "UIParticlePrivatePCH.h"
#include "Particle/EasyParticleState.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Rendering/DrawElements.h"
#include "Utility/UIParticleUtility.h"

#define MAX_PARTICLECOUNT 100000
#define MAX_LAYER 4

void FEasyParticleState::InitWithAsset(UUIParticleEmitterAsset* DataAsset)
{
	if (SelfAsset != DataAsset || DataAsset->ChildrenEmitters.Num() != ChildrenParticleArray.Num())
	{
		SelfAsset = DataAsset;
		ChildrenParticleArray.Empty();
		for (int32 i = 0; i < DataAsset->ChildrenEmitters.Num(); i++)
		{
			FEasyParticleChildEmitterArray temp;
			temp.ActivityInParentLifeTime = DataAsset->ChildrenEmitters[i].ActivityInParentLifeTime;
			temp.ChildrenAsset = DataAsset->ChildrenEmitters[i].ChildrenAsset;
			temp.IsStartEmitter = false;
			temp.LastFractionalNum = 0;
			temp.CountToSpawn = 0;
			temp.ZOrderOffset = DataAsset->ChildrenEmitters[i].ZOrderOffset;
			temp.FollowParentPosition = DataAsset->ChildrenEmitters[i].FollowParentPosition;
			temp.FollowParentSpeedPercent = DataAsset->ChildrenEmitters[i].FollowParentSpeedPercent;
			ChildrenParticleArray.Add(temp);
		}
	}
	else
	{
		for (int32 i = 0; i < DataAsset->ChildrenEmitters.Num(); i++)
		{
			FEasyParticleChildEmitterArray &temp = ChildrenParticleArray[i];
			temp.ActivityInParentLifeTime = DataAsset->ChildrenEmitters[i].ActivityInParentLifeTime;
			temp.ChildrenAsset = DataAsset->ChildrenEmitters[i].ChildrenAsset;
			temp.IsStartEmitter = false;
			temp.LastFractionalNum = 0;
			temp.CountToSpawn = 0;
			temp.ZOrderOffset = DataAsset->ChildrenEmitters[i].ZOrderOffset;
			temp.FollowParentPosition = DataAsset->ChildrenEmitters[i].FollowParentPosition;
			temp.FollowParentSpeedPercent = DataAsset->ChildrenEmitters[i].FollowParentSpeedPercent;
		}
	}
}

bool FEasyParticleState::IsParticleEnd()
{
	if (IsRoot)
	{
		return false;
	}
	else if (IsInPool())
	{
		return true;
	}
	else if (CurLifetime >= Deadtime || Deadtime <= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void FEasyParticleState::TrySpawnParticle( const FGeometry& AllottedGeometry)
{
	SCOPE_CYCLE_COUNTER(STAT_UIParticleSpawnParticle);
	for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
	{
		for (int32 j = 0; j < ChildrenParticleArray[i].ChildrenParticle.Num(); j++)
		{
			ChildrenParticleArray[i].ChildrenParticle[j]->TrySpawnParticle( AllottedGeometry );
		}
	}

	if (!IsEmitterEnd() && LayerOrder < MAX_LAYER)
	{
		for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
		{
			int32 iSpawnNum = ChildrenParticleArray[i].CountToSpawn;
			if (iSpawnNum > 0)
			{
				FUIParticleFactory::Instance()->CreatePool(RootParticle, iSpawnNum, LayerOrder);
				for (int32 num = 0; num < iSpawnNum; num++)
				{
					FEasyParticleState* particle = CreateParticle(AllottedGeometry, i);
				}
			}
		}
	}
}

FEasyParticleState* FEasyParticleState::CreateParticle(const FGeometry& AllottedGeometry, int32 ChildParticlePoolIndex)
{
	FEasyParticleChildEmitterArray &ChildEmitterArray = ChildrenParticleArray[ChildParticlePoolIndex];
	bool InFollowParentPosition = ChildEmitterArray.FollowParentPosition;
	float InFollowParentSpeedPercent = ChildEmitterArray.FollowParentSpeedPercent;
	int32 ZOrderOffset = ChildEmitterArray.ZOrderOffset;

	int32 lod = UUIParticleUtility::GetLOD();
	UUIParticleEmitterAsset* DataAsset = ChildEmitterArray.ChildrenAsset->GetLODAsset(lod);
	FEasyParticleState* Ret = nullptr;
	if (ChildEmitterArray.DeadParticleIndexPool.Num() > 0)
	{
		int32 index = ChildEmitterArray.DeadParticleIndexPool.Pop();
		if (index < ChildEmitterArray.ChildrenParticle.Num())
		{
			Ret = ChildEmitterArray.ChildrenParticle[index];
			if (Ret && Ret->IsInPool())
			{
			}
			else
			{
				ChildEmitterArray.DeadParticleIndexPool.Empty();
				for (int32 i = 0; i < ChildEmitterArray.ChildrenParticle.Num(); i++)
				{
					FEasyParticleState* Temp = ChildEmitterArray.ChildrenParticle[i];
					if (Temp->IsInPool())
					{
						ChildEmitterArray.DeadParticleIndexPool.Push(i);
					}
				}

				if (ChildEmitterArray.DeadParticleIndexPool.Num() > 0)
				{
					int32 index2 = ChildEmitterArray.DeadParticleIndexPool.Pop();
					Ret = ChildEmitterArray.ChildrenParticle[index2];
				}
			}
		}
	}
	if (Ret == nullptr)
	{
		Ret = FUIParticleFactory::Instance()->CreateParticle(RootParticle,LayerOrder);
		if (Ret == nullptr)
		{
			return nullptr;
		}
		ChildEmitterArray.ChildrenParticle.Add(Ret);
	}


	Ret->SetInPool(false);
	Ret->EmitterType = DataAsset->EmitterType;
	Ret->PositionType = DataAsset->PositionType;
	Ret->DrawEffect = DataAsset->DrawEffect;

	Ret->SizeLerpKey = FMath::FRandRange(0, 1);
	Ret->SizeXLerpKey.Random();
	Ret->SizeYLerpKey.Random();
	Ret->PositionXLerpKey = FMath::FRandRange(0, 1);
	Ret->PositionYLerpKey = FMath::FRandRange(0, 1);
	Ret->RotationSpeedLerpKey = FMath::FRandRange(0, 1);
	Ret->ColorLerpKey.Random();
	Ret->PivotLerpKey.Random();


	Ret->ParticleLifeSpanLerpKey = FMath::FRandRange(0, 1);
	Ret->RotationStartLerpKey = FMath::FRandRange(0, 1);
	Ret->GravityLerpKey.Random();
	Ret->StartSpeedLerpKey = FMath::FRandRange(0, 1);
	Ret->AirResistanceLerpKey = FMath::FRandRange(0, 1);
	Ret->RadialAccelerationLerpKey = FMath::FRandRange(0, 1);
	Ret->TangentialAccelerationLerpKey = FMath::FRandRange(0, 1);
	Ret->RadiusPercent = FMath::FRandRange(0, 1);
	Ret->DegreePerSecondPercent = FMath::FRandRange(0, 1);
	Ret->ScaleFollowSpeedVerticalLerpKey = FMath::FRandRange(0, 1);
	Ret->ScaleFollowSpeedDirectionLerpKey = FMath::FRandRange(0, 1);
	Ret->VerticalDirectionScaleLerpKey = FMath::FRandRange(0, 1);
	Ret->DirectionScaleLerpKey = FMath::FRandRange(0, 1);
	Ret->SineDirectionStartLerpKey = FMath::FRandRange(0, 1);
	Ret->SineDirectionSpeedLerpKey = FMath::FRandRange(0, 1);
	Ret->SpawnParticlePerSecondLerpKey = FMath::FRandRange(0, 1);
	Ret->MaxParticleCountLerpKey = FMath::FRandRange(0, 1);
	Ret->ParticleEmitAngleLerpKey = FMath::FRandRange(0, 1);
	Ret->EmitPositionLerpKey = FMath::FRandRange(0, 1);
	Ret->EmitPosRangeLerpKey = FMath::FRandRange(0, 1);

	Ret->ScalarParameterWhenStartLerpKeys.SetNum(DataAsset->ScalarParamsWhenStart.Num());
	for (int32 i = 0; i < Ret->ScalarParameterWhenStartLerpKeys.Num(); i++)
	{
		Ret->ScalarParameterWhenStartLerpKeys[i] = FMath::FRandRange(0, 1);
	}

	Ret->ScalarParameterLerpKeys.SetNum(DataAsset->ScalarParams.Num());
	for (int32 i = 0; i < Ret->ScalarParameterLerpKeys.Num(); i++)
	{
		Ret->ScalarParameterLerpKeys[i] = FMath::FRandRange(0, 1);
	}

	Ret->Deadtime = DataAsset->LifeSpan.GetValue_Float(0, 0, RootSpanTime, Ret->ParticleLifeSpanLerpKey); // DataAsset->LifeSpan.GetValue();
	Ret->RotationStart = DataAsset->RotationStart.GetValue_Float(0, 0, RootSpanTime, Ret->RotationStartLerpKey);
	Ret->RadialAcceleration = DataAsset->RadialAcceleration.GetValue_Float(0, 0, RootSpanTime, Ret->RadialAccelerationLerpKey);
	Ret->TangentialAcceleration = DataAsset->TangentialAcceleration.GetValue_Float(0, 0, RootSpanTime, Ret->TangentialAccelerationLerpKey);
	Ret->SineDirectionStart = DataAsset->SineDirectionStart.GetValue_Float(0, 0, RootSpanTime, SineDirectionStartLerpKey);
	Ret->SineDirectionRangePercent = FMath::FRandRange(0, 1);
	Ret->SineDirectionSpeed = DataAsset->SineDirectionSpeed.GetValue_Float(0, 0, RootSpanTime, SineDirectionSpeedLerpKey);
	Ret->AirResistance = DataAsset->AirResistance.GetValue_Float(0, 0, RootSpanTime, Ret->AirResistanceLerpKey);
	Ret->DirectionScale = DataAsset->DirectionScale.GetValue_Float(0, 0, RootSpanTime, Ret->DirectionScaleLerpKey);
	Ret->VerticalDirectionScale = DataAsset->VerticalDirectionScale.GetValue_Float(0, 0, RootSpanTime, Ret->VerticalDirectionScaleLerpKey);

	Ret->ScaleFollowSpeedVertical = DataAsset->ScaleFollowSpeedVertical.GetValue_Float(0, 0, RootSpanTime, Ret->ScaleFollowSpeedVerticalLerpKey);
	Ret->ScaleFollowSpeedDirection = DataAsset->ScaleFollowSpeedDirection.GetValue_Float(0, 0, RootSpanTime, Ret->ScaleFollowSpeedDirectionLerpKey);

	Ret->DegreePerSecond = DataAsset->DegreePerSecond.GetValue_Float(0, 0, RootSpanTime, Ret->DegreePerSecondPercent);

	Ret->ParticleEmitAngle = DataAsset->ParticleEmitAngle.GetValue_Float(0, 0, RootSpanTime, Ret->ParticleEmitAngleLerpKey);
	if (IsRoot && RootAsset && RootAsset->EmitAngleByWidgetAngle)
	{
		Ret->ParticleEmitAngle = Ret->ParticleEmitAngle + CurWidgetRotation * 180 / PI;
	}

	float RandomSpeed = DataAsset->StartSpeed.GetValue_Float(0, 0, RootSpanTime, Ret->StartSpeedLerpKey);

	Ret->Gravity = DataAsset->Gravity.GetValue_Vector2D(0, 0, RootSpanTime, Ret->GravityLerpKey);


	const FVector2D& LocalSize = AllottedGeometry.GetLocalSize();
	const FVector2D LocalPosition = LocalSize / 2;
	Ret->StartWidgetPositionWorld = AllottedGeometry.LocalToAbsolute(LocalPosition);

	if (PositionType == EPositionType::FREE)
	{
		Ret->StartPositionLocal = CurPos + ((StartWidgetPositionWorld - Ret->StartWidgetPositionWorld) / AllottedGeometry.Scale) + DataAsset->EmitPosition.GetValue(0, 0, RootSpanTime, Ret->EmitPositionLerpKey)* WholeScale;
	}
	else if (PositionType == EPositionType::RELATIVE)
	{
		Ret->StartPositionLocal = CurPos + DataAsset->EmitPosition.GetValue(0, 0, RootSpanTime, Ret->EmitPositionLerpKey)* WholeScale;
	}

	Ret->StartSpeed = FVector2D(1, 0).GetRotated(Ret->ParticleEmitAngle)*RandomSpeed;

	Ret->ImageBrush = FSlateBrush();
	Ret->ImageBrush.DrawAs = ESlateBrushDrawType::Image;

	if (DataAsset->ScalarParams.Num() > 0 || DataAsset->ScalarParamsWhenStart.Num() > 0)
	{
		UMaterialInterface * Material = Cast<UMaterialInterface>(DataAsset->ResourceObject);
		if (Material)
		{
			bool NeedCreate = true;
			if (Ret->ParticleResource != nullptr)
			{
				UMaterialInstanceDynamic * DynamicInstance = Cast<UMaterialInstanceDynamic>(Ret->ParticleResource);
				if (DynamicInstance != nullptr)
				{
					if (DynamicInstance->Parent == Material)
					{
						NeedCreate = false;
					}
				}
			}
			if (NeedCreate)
			{
				Ret->ParticleResource = UMaterialInstanceDynamic::Create(Material, NULL);
			}
		}
		else
		{
			Ret->ParticleResource = DataAsset->ResourceObject;
		}
	}
	else
	{
		Ret->ParticleResource = DataAsset->ResourceObject;
	}

	Ret->ImageBrush.SetResourceObject(Ret->ParticleResource);

	if (Ret->EmitterType == EEmitterType::Gravity)
	{
		Ret->SourcePos = DataAsset->EmitPosRange.GetValue();
	}
	else if (EmitterType == EEmitterType::Radial)
	{
		Ret->SourcePos = DataAsset->EmitPosRange.Min;
		Ret->Radius = DataAsset->Radius.GetValue_Float(0, 0, RootSpanTime, Ret->RadiusPercent);
	}
	else if (EmitterType == EEmitterType::Curve)
	{
		Ret->SourcePos = DataAsset->EmitPosRange.GetValue();
	}

	Ret->FollowParentPosition = InFollowParentPosition;
	Ret->FollowParentSpeedPercent = InFollowParentSpeedPercent;

	Ret->InitWithAsset(DataAsset);
	Ret->ParentParticle = this;
	Ret->RootParticle = RootParticle;

	Ret->RootSpanTime = RootSpanTime;
	Ret->ResetStateProperty();
	Ret->IsRoot = false;
	Ret->RootSpanTime = RootSpanTime;
	Ret->LayerOrder = LayerOrder + 1;
	Ret->ZOrder = ZOrder + ZOrderOffset;
	Ret->IsFirstTick = true;
	return Ret;
}


void FEasyParticleState::UpdateParticle(float dt, const FGeometry& AllottedGeometry)
{
	if (!SelfAsset || !SelfAsset->IsValidLowLevel())
	{
		StopEmit();
		return;
	}

	if (IsParticleEnd() && IsEmitterEnd())
	{
		return;
	}
	if (IsRoot && CurLifetime > Deadtime)
	{
		CurLifetime = 0;
		CurPercent = 0;
	}

	ImageBrush.DrawAs = ESlateBrushDrawType::Image;
	CurLifetime += dt;

	float NowPercent = CurLifetime / Deadtime;
	CurPercent = NowPercent;

	CurColor = SelfAsset->Color.GetValue_Color(CurPercent, CurLifetime, RootSpanTime, ColorLerpKey);

	if (SelfAsset->UseSeparateSize)
	{
		FVector2D CurSize_Local = SelfAsset->Size.GetValue_Vector2D(CurPercent, CurLifetime, RootSpanTime, SizeXLerpKey)*WholeScale;
		CurSizeX = CurSize_Local.X;
		CurSize_Local = SelfAsset->Size.GetValue_Vector2D(CurPercent, CurLifetime, RootSpanTime, SizeYLerpKey)*WholeScale;
		CurSizeY = CurSize_Local.Y;
	}
	else
	{
		CurSizeX = SelfAsset->Size.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, SizeLerpKey)*WholeScale;
		CurSizeY = CurSizeX;
	}

	CurPivot = SelfAsset->Pivot.GetValue_Vector2D(CurPercent, CurLifetime, RootSpanTime, PivotLerpKey);

	if (EmitterType == EEmitterType::Gravity)
	{

		RadialAcceleration = SelfAsset->RadialAcceleration.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, RadialAccelerationLerpKey);
		TangentialAcceleration = SelfAsset->TangentialAcceleration.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, TangentialAccelerationLerpKey);
		Gravity = SelfAsset->Gravity.GetValue_Vector2D(CurPercent, CurLifetime, RootSpanTime, GravityLerpKey);

		FVector2D RadialVector = (CurPos - SourcePos - StartPositionLocal);
		RadialVector.Normalize();
		FVector2D RadialForce = RadialVector * RadialAcceleration;
		FVector2D TangentialVector = FVector2D(-RadialVector.Y, RadialVector.X);
		FVector2D TangentialForce = TangentialVector * TangentialAcceleration;

		CurSpeed += (Gravity*dt);
		CurSpeed += (RadialForce*dt);
		CurSpeed += (TangentialForce*dt);

		AirResistance = SelfAsset->AirResistance.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, AirResistanceLerpKey);
		if (AirResistance != 0)
		{
			FVector2D DeleteSpeed = CurSpeed * AirResistance*dt;
			FVector2D AfterSpeed = CurSpeed - DeleteSpeed;

			if (AfterSpeed.X * CurSpeed.X <= 0)
			{
				CurSpeed.X = 0;
			}
			else
			{
				CurSpeed.X = AfterSpeed.X;
			}

			if (AfterSpeed.Y * CurSpeed.Y <= 0)
			{
				CurSpeed.Y = 0;
			}
			else
			{
				CurSpeed.Y = AfterSpeed.Y;
			}
		}

		FVector2D sinoffset = FVector2D(0, 0);
		{

			SineDirectionSpeed = SelfAsset->SineDirectionSpeed.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, SineDirectionSpeedLerpKey);
			FDateTime span = FDateTime::Now();
			float spansecond = ((span.GetSecond() * 1000 + (float)span.GetMillisecond())* SineDirectionSpeed) / 1000.0 + SineDirectionStart;
			float CurSineDirectionRange = SelfAsset->SineDirectionRange.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, SineDirectionRangePercent);
			float offset = (FMath::Sin(spansecond)*CurSineDirectionRange);
			FVector2D sineoffsetdirection = CurSpeed.GetRotated(90);
			sineoffsetdirection.Normalize();
			sinoffset = sineoffsetdirection * offset;

			CurSpeed += sinoffset;
		}

		CurPos += CurSpeed * dt  * WholeScale;
	}
	else if (EmitterType == EEmitterType::Radial)
	{
		float CurRadius = SelfAsset->Radius.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, RadiusPercent);
		float CurDegreePerSecond = SelfAsset->DegreePerSecond.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, DegreePerSecondPercent);

		CurAngle = CurAngle + CurDegreePerSecond * dt;
		FVector2D LastPos = CurPos;
		CurPos = FVector2D(1, 0).GetRotated(CurAngle) * CurRadius * WholeScale + StartPositionLocal;
		CurSpeed = (CurPos - LastPos) / dt;
	}
	else if (EmitterType == EEmitterType::Curve)
	{
		FVector2D LastPos = CurPos;
		CurPos = FVector2D(SelfAsset->PositionX.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, PositionXLerpKey), SelfAsset->PositionY.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, PositionYLerpKey))*WholeScale;
		CurSpeed = (CurPos - LastPos) / dt;
	}

	if (PositionType == EPositionType::FREE)
	{

	}
	else if (PositionType == EPositionType::RELATIVE)
	{
		if (SelfAsset && FollowParentPosition && ParentParticle)
		{
			if (EmitterType == EEmitterType::Gravity)
			{
				CurPos += ParentParticle->CurSpeed * dt;
			}
			else if (EmitterType == EEmitterType::Radial)
			{
				CurPos = FVector2D(1, 0).GetRotated(CurAngle) * Radius * dt + ParentParticle->CurPos;
			}
			else if (EmitterType == EEmitterType::Curve)
			{
				CurPos += ParentParticle->CurSpeed * dt;
			}
		}
	}

	if (SelfAsset->RotationFollowSpeed)
	{
		float SpeedAngle = 0;
		SpeedAngle = FMath::Atan2(CurSpeed.Y, CurSpeed.X);
		CurRotation = SpeedAngle * 180 / PI;
	}
	else
	{
		float NowRotationSpeed = SelfAsset->RotationSpeed.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, RotationSpeedLerpKey);
		CurRotation = CurRotation + NowRotationSpeed * dt;
	}


	if (SelfAsset->UseScaleFollowSpeedVertical)
	{
		ScaleFollowSpeedVertical = SelfAsset->ScaleFollowSpeedVertical.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, ScaleFollowSpeedVerticalLerpKey);
		VerticalDirectionScale = CurSpeed.Size() / 100.0 * ScaleFollowSpeedVertical;
	}
	else
	{
		VerticalDirectionScale = SelfAsset->VerticalDirectionScale.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, VerticalDirectionScaleLerpKey);
	}
	if (SelfAsset->UseScaleFollowSpeedDirection)
	{
		ScaleFollowSpeedDirection = SelfAsset->ScaleFollowSpeedDirection.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, ScaleFollowSpeedDirectionLerpKey);
		DirectionScale = CurSpeed.Size() / 100.0 * ScaleFollowSpeedDirection;
	}
	else
	{
		DirectionScale = SelfAsset->DirectionScale.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, DirectionScaleLerpKey);
	}
}

void FEasyParticleState::ResetStateProperty()
{
	SpawnParticleDeltaTime = 0;
	CurLifetime = 0;
	CurPercent = 0;
	if (SelfAsset->UseSeparateSize)
	{
		FVector2D CurSize_Local = SelfAsset->Size.GetValue_Vector2D(CurPercent, CurLifetime, RootSpanTime, SizeXLerpKey)*WholeScale;
		CurSizeX = CurSize_Local.X;
		CurSize_Local = SelfAsset->Size.GetValue_Vector2D(CurPercent, CurLifetime, RootSpanTime, SizeYLerpKey)*WholeScale;
		CurSizeY = CurSize_Local.Y;
	}
	else
	{
		CurSizeX = SelfAsset->Size.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, SizeLerpKey)*WholeScale;
		CurSizeY = CurSizeX;
	}

	CurPivot = SelfAsset->Pivot.GetValue_Vector2D(CurPercent, CurLifetime, RootSpanTime, PivotLerpKey);

	CurRotation = RotationStart;
	CurSpeed = StartSpeed;
	CurColor = SelfAsset->Color.GetValue_Color(CurPercent, CurLifetime, RootSpanTime, ColorLerpKey);
	CurAngle = ParticleEmitAngle;
	CurDelay = 0;
	bStopEmit = false;

	if (SelfAsset->ScalarParamsWhenStart.Num() > 0)
	{
		UMaterialInstanceDynamic *DynMat = Cast<UMaterialInstanceDynamic>(ImageBrush.GetResourceObject());
		if (DynMat)
		{
			for (int32 i = 0; i < SelfAsset->ScalarParamsWhenStart.Num(); i++)
			{
				FScalarParamCurve& Param = SelfAsset->ScalarParamsWhenStart[i];
				if (!Param.ScalarParamName.IsNone() && i < ScalarParameterWhenStartLerpKeys.Num())
				{
					float ParamLerpKey = ScalarParameterWhenStartLerpKeys[i];
					DynMat->SetScalarParameterValue(Param.ScalarParamName, Param.GetValue(CurPercent, CurLifetime, RootSpanTime, ParamLerpKey));
				}
			}
			DynMat->SetScalarParameterValue("LifeSpan", Deadtime);
		}
	}

	if (EmitterType == EEmitterType::Gravity)
	{
		CurPos = SourcePos + StartPositionLocal;
	}
	else if (EmitterType == EEmitterType::Radial)
	{
		CurPos = FVector2D(1, 0).GetRotated(ParticleEmitAngle)*Radius + StartPositionLocal;
	}
	else if (EmitterType == EEmitterType::Curve)
	{
		CurPos = SourcePos + StartPositionLocal + FVector2D(SelfAsset->PositionX.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, PositionXLerpKey), SelfAsset->PositionY.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, PositionYLerpKey))*WholeScale;
	}

	if (SelfAsset)
	{
		if (FollowParentSpeedPercent != 0 && ParentParticle)
		{
			CurSpeed = StartSpeed + ParentParticle->CurSpeed * FollowParentSpeedPercent;
		}
		else
		{
			CurSpeed = StartSpeed;
		}

		if (ParentParticle)
		{
			//StartPositionLocal = ParentParticle->CurPos + SelfAsset->EmitPosition.GetValue(ParentParticle->CurPercent,ParentParticle->CurLifetime,RootSpanTime, ParentParticle->EmitPositionLerpKey)* WholeScale;
		}
	}

	if (EmitterType == EEmitterType::Gravity)
	{
		CurPos = SourcePos + StartPositionLocal;
	}
	else if (EmitterType == EEmitterType::Radial)
	{
		CurPos = FVector2D(1, 0).GetRotated(ParticleEmitAngle)*Radius + StartPositionLocal;
	}
	else if (EmitterType == EEmitterType::Curve)
	{
		CurPos = SourcePos + StartPositionLocal + FVector2D(SelfAsset->PositionX.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, PositionXLerpKey), SelfAsset->PositionY.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, PositionYLerpKey))*WholeScale;
	}

	for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
	{
		ChildrenParticleArray[i].IsStartEmitter = false;
		ChildrenParticleArray[i].LastFractionalNum = 0;

		ChildrenParticleArray[i].DeadParticleIndexPool.SetNum(ChildrenParticleArray[i].ChildrenParticle.Num());
		for (int32 j = 0; j < ChildrenParticleArray[i].DeadParticleIndexPool.Num(); j++)
		{
			ChildrenParticleArray[i].DeadParticleIndexPool[j] = j;
			ChildrenParticleArray[i].ChildrenParticle[j]->SetInPool(true);
			ChildrenParticleArray[i].ChildrenParticle[j]->ResetStateProperty();
		}
	}
}

void FEasyParticleState::TickSelf(const float InDeltaTime, const FGeometry& AllottedGeometry, FVector2D InCurWidgetPosition, float InCurWidgetRotation, float RootScale, float InRootSpanTime)
{
	INC_DWORD_STAT(STAT_UIParticleCount);
	if (IsFirstTick)
	{
		IsFirstTick = false;
		TickSelf(0, AllottedGeometry, InCurWidgetPosition, InCurWidgetRotation);
	}
	if (CurDelay < TotalDelay)
	{
		CurDelay += InDeltaTime;
		return;
	}
	if (IsInPool())
	{
		return;
	}

	CurWidgetPosition = InCurWidgetPosition;
	CurWidgetRotation = InCurWidgetRotation;

	if (IsRoot)
	{
		const FVector2D& LocalSize = AllottedGeometry.GetLocalSize();
		StartWidgetPositionWorld = CurWidgetPosition;
		if (RootAsset && RootAsset->AutoEmitPosRange)
		{
			RootAsset->EmitPosRange.Min = LocalSize / 2 * -1;
			RootAsset->EmitPosRange.Max = LocalSize / 2;
		}

		if (RootAsset && RootAsset->AutoScale)
		{
			if (RootAsset->ScaleByX)
			{
				WholeScale = (LocalSize / RootAsset->DesignSize).X;
			}
			else
			{
				WholeScale = (LocalSize / RootAsset->DesignSize).Y;
			}
		}
		else
		{
			WholeScale = 1;
		}
		RootSpanTime = RootSpanTime + InDeltaTime;
	}
	else
	{
		RootSpanTime = InRootSpanTime;
		WholeScale = RootScale;
	}

	UpdateParticle(InDeltaTime, AllottedGeometry);

	for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
	{
		int32 TotleCount = ChildrenParticleArray[i].ChildrenParticle.Num();
		for (int32 j = 0; j < TotleCount; j++)
		{
			//TSharedPtr<FEasyParticleState, ESPMode::ThreadSafe> Val = ChildrenParticleArray[i].ChildrenParticle[j];
			FEasyParticleState* Val = ChildrenParticleArray[i].ChildrenParticle[j];
			if (Val->IsAllChildrenEnd() && Val->IsEmitterEnd() && !Val->IsInPool())
			{
				Val->SetInPool(true);
				ChildrenParticleArray[i].DeadParticleIndexPool.Push(j);
			}
		}
	}
	SpawnParticleDeltaTime = SpawnParticleDeltaTime + InDeltaTime;
	CaculateSpawnParticleCount();
}

void FEasyParticleState::CaculateSpawnParticleCount()
{
	if (!IsEmitterEnd() && LayerOrder < MAX_LAYER)
	{
		for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
		{
			if (Deadtime == 0)
			{
				break;
			}
			if ((CurLifetime / Deadtime) >= ChildrenParticleArray[i].ActivityInParentLifeTime && ChildrenParticleArray[i].ChildrenAsset)
			{
				int32 lod = UUIParticleUtility::GetLOD();
				UUIParticleEmitterAsset* ChildrenAsset = ChildrenParticleArray[i].ChildrenAsset->GetLODAsset(lod);
				if (!ChildrenParticleArray[i].IsStartEmitter)
				{
					ChildrenParticleArray[i].EmitterStartTime = FDateTime::Now();
					ChildrenParticleArray[i].IsStartEmitter = true;
				}

				if (ChildrenParticleArray[i].IsStartEmitter)
				{
					FTimespan span = FDateTime::Now() - ChildrenParticleArray[i].EmitterStartTime;
					float spansecond = span.GetTotalMilliseconds() / 1000;
					if (ChildrenAsset->EmitSeconds != -1 && spansecond >= ChildrenAsset->EmitSeconds)
					{
						break;
					}
				}

				float MaxCount = ChildrenAsset->MaxParticleCount.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, MaxParticleCountLerpKey);
				float MaxCanSpawn = MaxCount - ChildrenParticleArray[i].GetActivitedParticleNum();

				float CurSpawnParticlePerSecond = ChildrenAsset->SpawnParticlePerSecond.GetValue_Float(CurPercent, CurLifetime, RootSpanTime, SpawnParticlePerSecondLerpKey);
				float fSpawnNum = SpawnParticleDeltaTime * CurSpawnParticlePerSecond;
				int32 iSpawnNum = fSpawnNum + ChildrenParticleArray[i].LastFractionalNum;

				iSpawnNum = iSpawnNum <= MaxCanSpawn ? iSpawnNum : MaxCanSpawn;
				ChildrenParticleArray[i].LastFractionalNum = fSpawnNum + ChildrenParticleArray[i].LastFractionalNum - (float)iSpawnNum;
				ChildrenParticleArray[i].CountToSpawn = iSpawnNum;
			}
		}
	}
	SpawnParticleDeltaTime = 0;
}

void FEasyParticleState::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled)
{	
	if (IsNeedPaint)
	{
		if (SelfAsset->ScalarParams.Num() > 0)
		{
			UMaterialInstanceDynamic *DynMat = Cast<UMaterialInstanceDynamic>(ImageBrush.GetResourceObject());
			if (DynMat)
			{
				for (int32 i = 0; i < SelfAsset->ScalarParams.Num(); i++)
				{
					FScalarParamCurve& Param = SelfAsset->ScalarParams[i];
					if (!Param.ScalarParamName.IsNone() && i < ScalarParameterLerpKeys.Num())
					{
						float ParamLerpKey = ScalarParameterLerpKeys[i];
						DynMat->SetScalarParameterValue(Param.ScalarParamName, Param.GetValue(CurPercent, CurLifetime, RootSpanTime, ParamLerpKey));
					}
				}
			}
		}
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + ZOrder, PaintGeometryCache, &(this->ImageBrush), (ESlateDrawEffect)DrawEffect, PaintColorCache);
	}

	for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
	{
		for (int32 j = 0; j < ChildrenParticleArray[i].ChildrenParticle.Num(); j++)
		{
			ChildrenParticleArray[i].ChildrenParticle[j]->OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
		}
	}
}

void FEasyParticleState::OnPaintSelf(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled)
{	
	if (IsNeedPaint)
	{
		if (SelfAsset->ScalarParams.Num() > 0)
		{
			UMaterialInstanceDynamic *DynMat = Cast<UMaterialInstanceDynamic>(ImageBrush.GetResourceObject());
			if (DynMat)
			{
				for (int32 i = 0; i < SelfAsset->ScalarParams.Num(); i++)
				{
					FScalarParamCurve& Param = SelfAsset->ScalarParams[i];
					if (!Param.ScalarParamName.IsNone() && i < ScalarParameterLerpKeys.Num())
					{
						float ParamLerpKey = ScalarParameterLerpKeys[i];
						DynMat->SetScalarParameterValue(Param.ScalarParamName, Param.GetValue(CurPercent, CurLifetime, RootSpanTime, ParamLerpKey));
					}
				}
			}
		}
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + ZOrder, PaintGeometryCache, &(this->ImageBrush), (ESlateDrawEffect)DrawEffect, PaintColorCache);
	}
}

void FEasyParticleState::CaculatePaintParams(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FWidgetStyle& InWidgetStyle)
{
	IsNeedPaint = false;
	if (IsInPool())
	{
		return;
	}
	if (!this->IsRoot && this->ImageBrush.DrawAs != ESlateBrushDrawType::NoDrawType && this->ImageBrush.GetResourceObject())
	{
		const FLinearColor FinalColorAndOpacity(InWidgetStyle.GetColorAndOpacityTint() * this->CurColor * this->ImageBrush.GetTint(InWidgetStyle));
		FVector2D BasePos = FVector2D::ZeroVector;
		if (PositionType == EPositionType::RELATIVE)
		{
			BasePos = CurWidgetPosition;
		}
		else if (PositionType == EPositionType::FREE)
		{
			BasePos = StartWidgetPositionWorld;
		}
		float Rotation = this->CurRotation / 180 * PI;

		const FSlateRenderTransform RenderTransform = AllottedGeometry.GetAccumulatedRenderTransform();
		float A, B, C, D;
		RenderTransform.GetMatrix().GetMatrix(A, B, C, D);
		float ParentAngle = FMath::Atan2(B, A);

		FSlateRenderTransform tranform(::Concatenate(FScale2D(DirectionScale, VerticalDirectionScale), FQuat2D(Rotation - ParentAngle)));

		float Cosvalue = A / FMath::Sqrt((B*B + D * D));
		float Sinvalue = FMath::Sqrt(1 - Cosvalue * Cosvalue) * (B > 0 ? 1 : -1);

		FVector2D Scale2D = RenderTransform.GetMatrix().GetScale().GetVector();

		FVector2D LocalPosition = this->CurPos - FVector2D(this->CurSizeX * Cosvalue - this->CurSizeY * Sinvalue, this->CurSizeY * Cosvalue + this->CurSizeX * Sinvalue) / 2;
		FVector2D WindowOffset = Args.GetWindowToDesktopTransform();

		FVector2D FixPosition = AllottedGeometry.AbsoluteToLocal(LocalPosition * Scale2D + BasePos - WindowOffset);
		FSlateLayoutTransform layouttransform = FSlateLayoutTransform(1.0, FixPosition);
		FGeometry childgeometry = AllottedGeometry.MakeChild(FVector2D(this->CurSizeX, this->CurSizeY), layouttransform, tranform, CurPivot);

		if (!IsParticleEnd())
		{
			IsNeedPaint = true;
			PaintGeometryCache = childgeometry.ToPaintGeometry();
			PaintColorCache = FinalColorAndOpacity;
		}
	}
}

void FEasyParticleState::StopEmit()
{
	bStopEmit = true;
}

bool FEasyParticleState::IsEmitterEnd()
{
	if (ChildrenParticleArray.Num() == 0 || bStopEmit || IsParticleEnd())
	{
		return true;
	}
	else
	{
		for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
		{
			if (!ChildrenParticleArray[i].IsStartEmitter)
			{
				return false;
			}
			else
			{
				FTimespan span = FDateTime::Now() - ChildrenParticleArray[i].EmitterStartTime;
				float spansecond = span.GetTotalMilliseconds() / 1000;

				int32 lod = UUIParticleUtility::GetLOD();
				UUIParticleEmitterAsset* ChildrenAsset = ChildrenParticleArray[i].ChildrenAsset->GetLODAsset(lod);
				if (spansecond < ChildrenAsset->EmitSeconds || ChildrenAsset->EmitSeconds == -1)
				{
					return false;
				}
			}
		}
	}
	return true;
}

bool FEasyParticleState::IsAllChildrenEnd()
{
	if (!IsParticleEnd() && !IsRoot)
	{
		return false;
	}
	else
	{
		for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
		{
			for (int32 j = 0; j < ChildrenParticleArray[i].ChildrenParticle.Num(); j++)
			{
				if (!ChildrenParticleArray[i].ChildrenParticle[j]->IsAllChildrenEnd())
				{
					return false;
				}
			}
		}
	}
	return true;
}

int FEasyParticleState::GetChildrenParticleCount()
{
	int ret = 0;
	if (!IsRoot)
	{
		ret = IsInPool() ? 0 : 1;
	}
	for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
	{
		for (int32 j = 0; j < ChildrenParticleArray[i].ChildrenParticle.Num(); j++)
		{
			ret += ChildrenParticleArray[i].ChildrenParticle[j]->GetChildrenParticleCount();
		}
	}
	return ret;
}
int FEasyParticleState::GetChildrenParticleCountInPool()
{
	int ret = 0;
	if (!IsRoot)
	{
		ret = IsInPool() ? 1 : 0;
	}
	for (int32 i = 0; i < ChildrenParticleArray.Num(); i++)
	{
		for (int32 j = 0; j < ChildrenParticleArray[i].ChildrenParticle.Num(); j++)
		{
			ret += ChildrenParticleArray[i].ChildrenParticle[j]->GetChildrenParticleCountInPool();
		}
	}
	return ret;
}
bool FEasyParticleState::IsInPool()
{
	return InPool;
}

void FEasyParticleState::SetInPool(bool inpool)
{
	InPool = inpool;
}

FEasyParticleState::FEasyParticleState()
{
	SpawnParticleDeltaTime = 0;

	CurLifetime = 0;
	CurPercent = 0;
	CurSize = 0;
	CurSizeX = 0;
	CurSizeY = 0;
	CurRotation = 0;
	CurAngle = 0;

	CurSpeed = FVector2D(0, 0);
	CurPos = FVector2D(0, 0);
	CurPivot = FVector2D(0, 0);
	CurColor = FLinearColor(0, 0, 0, 0);

	CurDelay = 0;
	RootSpanTime = 0;
	WholeScale = 1;
	ScaleFollowSpeedDirection = 0;
	ScaleFollowSpeedVertical = 0;
	CurWidgetPosition = FVector2D(0, 0);
	CurWidgetRotation = 0;
	ChildrenParticleArray.Empty();
	IsFirstTick = true;
	IsNeedPaint = false;

	/***************RandomKey ,Set in Create*************/
	Deadtime = 0;
	ParticleEmitAngle = 0;
	RotationStart = 0;
	DirectionScale = 0;
	VerticalDirectionScale = 0;

	EmitterType = EEmitterType::Curve;
	PositionType = EPositionType::FREE;
	DrawEffect = EParticleDrawEffect::None;
	StartSpeed = FVector2D(0, 0);
	FollowParentPosition = false;
	FollowParentSpeedPercent = 0;

	//LerpKey//
	SizeLerpKey = 0;
	SizeXLerpKey.SetValue(0, 0);
	SizeYLerpKey.SetValue(0, 0);
	RotationStartLerpKey = 0;
	RotationSpeedLerpKey = 0;
	ColorLerpKey.SetValue(0, 0, 0, 0);
	PivotLerpKey.SetValue(0, 0);
	PositionXLerpKey = 0;
	PositionYLerpKey = 0;
	ParticleLifeSpanLerpKey = 0;
	GravityLerpKey.SetValue(0, 0);
	StartSpeedLerpKey = 0;
	AirResistanceLerpKey = 0;
	RadialAccelerationLerpKey = 0;
	TangentialAccelerationLerpKey = 0;
	ScaleFollowSpeedDirectionLerpKey = 0;
	ScaleFollowSpeedVerticalLerpKey = 0;
	VerticalDirectionScaleLerpKey = 0;
	DirectionScaleLerpKey = 0;
	SineDirectionStartLerpKey = 0;
	SineDirectionSpeedLerpKey = 0;
	SineDirectionRangePercent = 0;
	SpawnParticlePerSecondLerpKey = 0;
	MaxParticleCountLerpKey = 0;
	ParticleEmitAngleLerpKey = 0;
	ScalarParameterLerpKeys.Empty();
	ScalarParameterWhenStartLerpKeys.Empty();
	EmitPositionLerpKey = 0;
	EmitPosRangeLerpKey = 0;

	//Gravity_Configuration//
	Gravity = FVector2D(0, 0);
	SourcePos = FVector2D(0, 0);
	RadialAcceleration = 0;
	TangentialAcceleration = 0;
	SineDirectionStart = 0;
	SineDirectionSpeed = 0;
	AirResistance = 0;
	TotalDelay = 0;
	LayerOrder = 0;
	ZOrder = 0;

	//Radial_Configuration//
	Radius = 0;
	DegreePerSecond = 0;
	RadiusPercent = 0;
	DegreePerSecondPercent = 0;

	/***************Particle Info ,Set in Create*************/
	ImageBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	ParticleResource = nullptr;
	RootAsset = nullptr;
	SelfAsset = nullptr;

	IsRoot = false;
	bStopEmit = false;
	InPool = false;

	StartPositionLocal = FVector2D(0, 0);
	StartWidgetPositionWorld = FVector2D(0, 0);
	ParentParticle = nullptr;
	RootParticle = nullptr;
}
FEasyParticleState::~FEasyParticleState()
{
}

int32 FEasyParticleChildEmitterArray::GetActivitedParticleNum()
{
	int ret = 0;
	for (int32 i = 0; i < ChildrenParticle.Num(); i++)
	{
		if (!ChildrenParticle[i]->IsParticleEnd())
		{
			ret++;
		}
	}
	return ret;
}

/************** Particle Root State **************/

FEasyParticleRootState::FEasyParticleRootState()
{
	FirstRootTick = false;
	IsRoot = true;
}

FEasyParticleRootState::~FEasyParticleRootState()
{
	FUIParticleFactory::Instance()->OnRootDestroy(this);
}

void FEasyParticleRootState::FirstTick(const float FirstDeltaTime, const FGeometry& AllottedGeometry )
{
	if (FirstRootTick && IsRoot)
	{
		FirstRootTick = false;
		TickTick(FirstDeltaTime, AllottedGeometry );
	}
}

void FEasyParticleRootState::TickTick(const float FirstDeltaTime, const FGeometry& AllottedGeometry )
{
	if (this->IsRoot && this->RootAsset && this->RootAsset->StartTimeOffset > 0 && this->RootAsset->StartTimeOffset > FirstDeltaTime && this->RootAsset->SamplingTimes > 0)
	{
		float steptime = (this->RootAsset->StartTimeOffset - FirstDeltaTime) / this->RootAsset->SamplingTimes;
		for (int32 i = 0; i < this->RootAsset->SamplingTimes; i++)
		{
			FUIParticleFactory::Instance()->TickRoot(this, steptime, AllottedGeometry);
			TrySpawnParticle(AllottedGeometry);
		}
	}
}


void FEasyParticleRootState::ResetRoot()
{
	FEasyParticleState::ResetStateProperty();
	RootSpanTime = 0;
	EmitPositionLerpKey = FMath::FRandRange(0, 1);
	if (SelfAsset)
	{
		CurSpeed = StartSpeed;
		StartPositionLocal = SelfAsset->EmitPosition.GetValue(0, 0, RootSpanTime, EmitPositionLerpKey)* WholeScale;
	}

	if (EmitterType == EEmitterType::Gravity)
	{
		CurPos = SourcePos + StartPositionLocal;
	}
	else if (EmitterType == EEmitterType::Radial)
	{
		CurPos = FVector2D(1, 0).GetRotated(ParticleEmitAngle)*Radius + StartPositionLocal;
	}
	else if (EmitterType == EEmitterType::Curve)
	{
		CurPos = SourcePos + StartPositionLocal + FVector2D(SelfAsset->PositionX.GetValue_Float(0, 0, RootSpanTime, PositionXLerpKey), SelfAsset->PositionY.GetValue_Float(0, 0, RootSpanTime, PositionYLerpKey))*WholeScale;
	}
	LayerOrder = 0;
	Deadtime = ROOTLIFE;
	FirstRootTick = true;
	RootParticle = this;
}



void FEasyParticleRootState::InitRoot(UUIParticleEmitterAsset* RootChildAsset, float ActiveDelay, int32 RootZOrder)
{
	if (RootChildAsset)
	{
		UUIParticleEmitterAsset * TempAsset = nullptr;
		if (RootAsset != RootChildAsset)
		{
			RootAsset = RootChildAsset;
			TempAsset = NewObject<UUIParticleEmitterAsset>();
			TempAsset->Reset();
			FChildEmitter temp;
			temp.ActivityInParentLifeTime = 0;
			temp.ChildrenAsset = RootChildAsset;
			temp.FollowParentPosition = false;
			temp.FollowParentSpeedPercent = 0;
			temp.ZOrderOffset = RootZOrder;
			TempAsset->ChildrenEmitters.Add(temp);
			FUIParticleFactory::Instance()->ClearChildren(this);
		}
		else
		{
			TempAsset = SelfAsset;
			if (TempAsset->ChildrenEmitters.Num() == 1)
			{
				TempAsset->ChildrenEmitters[0].ZOrderOffset = RootZOrder;
			}
		}

		Deadtime = ROOTLIFE;
		IsRoot = true;
		SetInPool(false);

		SizeLerpKey = FMath::FRandRange(0, 1);
		SizeXLerpKey.Random();
		SizeYLerpKey.Random();
		PositionXLerpKey = FMath::FRandRange(0, 1);
		PositionYLerpKey = FMath::FRandRange(0, 1);
		RotationSpeedLerpKey = FMath::FRandRange(0, 1);
		ColorLerpKey.Random();
		PivotLerpKey.Random();

		ParticleLifeSpanLerpKey = FMath::FRandRange(0, 1);
		RotationStartLerpKey = FMath::FRandRange(0, 1);
		GravityLerpKey.Random();
		StartSpeedLerpKey = FMath::FRandRange(0, 1);
		AirResistanceLerpKey = FMath::FRandRange(0, 1);
		RadialAccelerationLerpKey = FMath::FRandRange(0, 1);
		TangentialAccelerationLerpKey = FMath::FRandRange(0, 1);
		RadiusPercent = FMath::FRandRange(0, 1);
		DegreePerSecondPercent = FMath::FRandRange(0, 1);
		ScaleFollowSpeedVerticalLerpKey = FMath::FRandRange(0, 1);
		ScaleFollowSpeedDirectionLerpKey = FMath::FRandRange(0, 1);
		VerticalDirectionScaleLerpKey = FMath::FRandRange(0, 1);
		DirectionScaleLerpKey = FMath::FRandRange(0, 1);
		SineDirectionStartLerpKey = FMath::FRandRange(0, 1);
		SineDirectionSpeedLerpKey = FMath::FRandRange(0, 1);
		SpawnParticlePerSecondLerpKey = FMath::FRandRange(0, 1);
		MaxParticleCountLerpKey = FMath::FRandRange(0, 1);

		RootSpanTime = 0;
		TotalDelay = ActiveDelay;
		InitWithAsset(TempAsset);
		ResetRoot();
	}
	else
	{
		StopEmit();
	}
}


/**************  Particle Group State **************/

FEasyParticleGroupState::FEasyParticleGroupState()
{
}

FEasyParticleGroupState::~FEasyParticleGroupState()
{
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			FUIParticleFactory::Instance()->OnRootDestroy(Emitter.Get());
		}
	}
}


bool FEasyParticleGroupState::IsEnd()
{
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			if (!Emitter->IsEmitterEnd() || !Emitter->IsAllChildrenEnd())
			{
				return false;
			}
		}
	}
	return true;
}

void FEasyParticleGroupState::InitWithAsset(UUIParticleAsset* Asset)
{
	if (Asset)
	{
		int32 index = 0;
		for (auto EmitterInfo : Asset->Emitters)
		{
			if (EmitterInfo.Asset && !EmitterInfo.Disable)
			{
				if (index < Emitters.Num())
				{
					TSharedPtr<FEasyParticleRootState, ESPMode::ThreadSafe> RootEmitter = Emitters[index];
					if (!RootEmitter.IsValid())
					{
						RootEmitter = MakeShareable(new FEasyParticleRootState());
						FUIParticleFactory::Instance()->RefRoot(RootEmitter);
						Emitters[index] = RootEmitter;
					}
					RootEmitter->InitRoot(EmitterInfo.Asset, EmitterInfo.ActiveDelay, EmitterInfo.ZOrder);
				}
				else
				{
					TSharedPtr<FEasyParticleRootState, ESPMode::ThreadSafe> RootEmitter = MakeShareable(new FEasyParticleRootState());
					Emitters.Add(RootEmitter);
					FUIParticleFactory::Instance()->RefRoot(RootEmitter);
					RootEmitter->InitRoot(EmitterInfo.Asset, EmitterInfo.ActiveDelay, EmitterInfo.ZOrder);
				}
				index++;
			}
		}
		Emitters.SetNum(index, true);
	}
}


int FEasyParticleGroupState::GetParticleCount()
{
	int32 ret = 0;
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			ret += Emitter->GetChildrenParticleCount();
		}
	}
	return ret;
}

int FEasyParticleGroupState::GetParticleCountInPool()
{
	int32 ret = 0;
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			ret += Emitter->GetChildrenParticleCountInPool();
		}
	}
	return ret;
}

bool FEasyParticleGroupState::Play()
{
	bool ret = false;
	FirstRootTick = true;
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			Emitter->ResetRoot();
			ret = true;
		}
	}
	return ret;
}

void FEasyParticleGroupState::Stop()
{
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			Emitter->ResetRoot();
		}
	}
}

void FEasyParticleGroupState::StopEmit()
{
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			Emitter->StopEmit();
		}
	}
}

void FEasyParticleGroupState::Tick(const float InDeltaTime, const FGeometry& AllottedGeometry)
{
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			FUIParticleFactory::Instance()->TickRoot(Emitter.Get(), InDeltaTime, AllottedGeometry);
		}
	}
}
void FEasyParticleGroupState::TrySpawnParticle(const FGeometry& AllottedGeometry)
{
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			Emitter->TrySpawnParticle(AllottedGeometry);
		}
	}
}

void FEasyParticleGroupState::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled)
{
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			FUIParticleFactory::Instance()->OnRootPaint(Emitter.Get(), Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
		}
	}
}

void FEasyParticleGroupState::FirstTick(const float FirstDeltaTime, const FGeometry& AllottedGeometry)
{
	FirstRootTick = false;
	for (auto Emitter : Emitters)
	{
		if (Emitter.IsValid())
		{
			Emitter->FirstTick(FirstDeltaTime, AllottedGeometry);
		}
	}
}

