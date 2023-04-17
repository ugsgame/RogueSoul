// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Asset/UIParticleEmitterAsset.h"
#include "Particle/EasyParticleState.h"
#include "UObject/GCObject.h"
#include "UIParticleUtility.generated.h"

#define MIN_POOL_SIZE 10

UCLASS(BlueprintType)
class UIPARTICLE_API  UUIParticleUtility : public UObject
{
	GENERATED_UCLASS_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = Particle)
		static void SetLOD(int32 newlod);
	UFUNCTION(BlueprintCallable, Category = Particle)
		static int32 GetLOD();

	UFUNCTION(BlueprintCallable, Category = Particle)
		static void SetMultiThread(bool value);
	UFUNCTION(BlueprintCallable, Category = Particle)
		static bool GetMultiThread();

private:
	static int32 LevelOfDetail;
	static bool Multi_Thread;
};



class FEasyParticleStateBatch
{
public:
	FEasyParticleState* Pool;
	int32 Count;
	int32 Size;
	FEasyParticleStateBatch(int32 NeedCount)
	{
		if (NeedCount < MIN_POOL_SIZE / 2)
		{
			Count = 0;
			Size = MIN_POOL_SIZE;
		}
		else
		{
			Size = NeedCount * 10;
			Count = 0;
		}
		//Size = 1;
		Pool = new FEasyParticleState[Size];
	}
	~FEasyParticleStateBatch()
	{
		delete[]Pool;
	}
	bool HasFree()
	{
		return Count < Size;
	}

	bool HasFree(int32 NeedCount)
	{
		return Size - Count >= NeedCount;
	}

	int32 FreeCount()
	{
		return Size - Count;
	}

	FEasyParticleState* GetFree()
	{
		if (HasFree())
		{
			Count = Count + 1;
			return Pool + Count - 1;
		}
		return nullptr;
	}

	int32 Num()
	{
		return Count;
	}

	FEasyParticleState* operator[](int pos)
	{
		FEasyParticleState * Ret = nullptr;
		if (pos < 0)
			return nullptr;
		if (pos >= Num())
			return nullptr;
		return Pool + pos;
	}
};
class FEasyParticleStatePool
{
public:
	TArray<FEasyParticleStateBatch*> PoolList;
	int32 CurIndex;
	int32 FreeCount;
	int32 TotalCount;

	FEasyParticleStatePool()
	{
		CurIndex = -1;
		FreeCount = 0;
		TotalCount = 0;
	}

	~FEasyParticleStatePool()
	{
		for (int32 i = 0; i < PoolList.Num(); i++)
		{
			FEasyParticleStateBatch* CurBatch = PoolList[i];
			if (CurBatch)
			{
				delete CurBatch;
			}
		}
	}

	bool HasFree(int32 NeedCount)
	{
		if (NeedCount <= 0)
		{
			return true;
		}
		return NeedCount <= FreeCount;
	}

	void CreatePool(int32 NeedCount)
	{
		if (!HasFree(NeedCount))
		{
			FEasyParticleStateBatch* NewBatch = new FEasyParticleStateBatch(NeedCount);
			PoolList.Push(NewBatch);
			FreeCount = FreeCount + NewBatch->FreeCount();
			TotalCount = TotalCount + NewBatch->FreeCount();
		}
		if (PoolList.Num() == 1)
		{
			CurIndex = 0;
		}
	}

	FEasyParticleState* GetFree()
	{
		if (CurIndex == -1)
		{
			CreatePool(MIN_POOL_SIZE);
		}
		FEasyParticleStateBatch* CurBatch = PoolList[CurIndex];
		if (!CurBatch->HasFree())
		{
			if (CurIndex >= (PoolList.Num() - 1))
			{
				CreatePool(MIN_POOL_SIZE);
			}
			CurIndex++;
			CurBatch = PoolList[CurIndex];
		}
		if (CurBatch)
		{
			FEasyParticleState* Ret = CurBatch->GetFree();
			if (Ret)
			{
				FreeCount--;
			}
			return Ret;
		}
		return nullptr;
	}

	int32 Num()
	{
		return TotalCount - FreeCount;
	}

	FEasyParticleState* operator[](int pos)
	{
		if (pos < 0)
			return nullptr;
		if (pos >= Num())
			return nullptr;
		int32 indexlocal = pos;
		for (int32 i = 0; i < PoolList.Num(); i++)
		{
			FEasyParticleStateBatch* CurBatch = PoolList[i];
			if (CurBatch)
			{
				if (indexlocal >= CurBatch->Num())
				{
					indexlocal = indexlocal - CurBatch->Num();
				}
				else
				{
					return (*CurBatch)[indexlocal];
				}
			}
		}
		return nullptr;
	}
};


class UIPARTICLE_API  FUIParticleFactory : public FGCObject
{
public:

	static FUIParticleFactory* Instance();

	void RefRoot(TSharedPtr<FEasyParticleRootState, ESPMode::ThreadSafe> RootPtr);
	void CreatePool(FEasyParticleRootState* RootPtr, int32 NeedCount, int32 Layer);
	FEasyParticleState* CreateParticle(FEasyParticleRootState* RootPtr,int32 Layer);
	void ClearChildren(FEasyParticleRootState* RootPtr);
	void OnRootDestroy(FEasyParticleRootState* RootPtr);

	int32 GetChildrenCount(FEasyParticleRootState* RootPtr);
	void TickRoot(FEasyParticleRootState* RootPtr, const float InDeltaTime, const FGeometry& AllottedGeometry);
	void OnRootPaint(FEasyParticleRootState* RootPtr, const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled);

	void AddReferencedMaterialInstances(FEasyParticleState* Emitter, FReferenceCollector& Collector);
	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

private:
	void ParallelTick(FEasyParticleRootState* RootPtr, const float InDeltaTime, const FGeometry& AllottedGeometry);
	void Tick(FEasyParticleRootState* RootPtr, const float InDeltaTime, const FGeometry& AllottedGeometry);
	void OnPaint(FEasyParticleRootState* RootPtr, const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled);
	void OnParallelPaint(FEasyParticleRootState* RootPtr, const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled);

	TArray<TMap<FEasyParticleRootState*, FEasyParticleStatePool*>> ParticlePoolMapArray;
	TArray<TSharedPtr<FEasyParticleRootState, ESPMode::ThreadSafe>> ParticleRefArray;

private:
	FUIParticleFactory();
	static FUIParticleFactory* m_Instance;

};
