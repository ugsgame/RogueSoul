// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "Utility/UIParticleUtility.h"
#include "UIParticlePrivatePCH.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Utility/UIParticleUtility.h"
#include "Async/TaskGraphInterfaces.h"
#include "Async/ParallelFor.h"

#define MIN_COUNT_FOR_MULTITHREAD 200
#define COUNT_PARALLEL 100

#define MIN_COUNT_FOR_MULTITHREAD_PAINT 1000
#define COUNT_PARALLEL_PAINT 500

int32 UUIParticleUtility::LevelOfDetail = -1;
bool UUIParticleUtility::Multi_Thread = false;
FUIParticleFactory* FUIParticleFactory::m_Instance = nullptr;


UUIParticleUtility::UUIParticleUtility(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

void UUIParticleUtility::SetLOD(int32 newlod)
{
	LevelOfDetail = newlod;
}

int32 UUIParticleUtility::GetLOD()
{
	return LevelOfDetail;
}


void UUIParticleUtility::SetMultiThread(bool value)
{
	Multi_Thread = value;
}

bool UUIParticleUtility::GetMultiThread()
{
	return Multi_Thread;
}

FUIParticleFactory* FUIParticleFactory::Instance()
{
	if (m_Instance == nullptr)
	{
		m_Instance = new FUIParticleFactory();
	}
	return m_Instance;
}
FUIParticleFactory::FUIParticleFactory()
{
	ParticlePoolMapArray.Empty();
	ParticlePoolMapArray.Add(TMap<FEasyParticleRootState*, FEasyParticleStatePool*>());
	ParticlePoolMapArray.Add(TMap<FEasyParticleRootState*, FEasyParticleStatePool*>());
	ParticlePoolMapArray.Add(TMap<FEasyParticleRootState*, FEasyParticleStatePool*>());
	ParticlePoolMapArray.Add(TMap<FEasyParticleRootState*, FEasyParticleStatePool*>());

	ParticleRefArray = TArray<TSharedPtr<FEasyParticleRootState, ESPMode::ThreadSafe>>();
}

FEasyParticleState* FUIParticleFactory::CreateParticle(FEasyParticleRootState* RootPtr,int32 Layer)
{
	if (RootPtr)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr == nullptr)
		{
			FEasyParticleStatePool* pArray = new FEasyParticleStatePool();
			ParticlePoolMapArray[Layer].Add(RootPtr, pArray);
			pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		}
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePoolPtr = *pParticlePoolPtr;
			if (ParticlePoolPtr)
			{
				return ParticlePoolPtr->GetFree();
			}
		}
	}
	return nullptr;
}
void FUIParticleFactory::CreatePool(FEasyParticleRootState* RootPtr, int32 NeedCount,int32 Layer)
{
	if (RootPtr)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr == nullptr)
		{
			FEasyParticleStatePool* pArray = new FEasyParticleStatePool();
			ParticlePoolMapArray[Layer].Add(RootPtr, pArray);
			pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		}
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePoolPtr = *pParticlePoolPtr;
			if (ParticlePoolPtr)
			{
				ParticlePoolPtr->CreatePool(NeedCount);
			}
		}
	}
}
void FUIParticleFactory::RefRoot(TSharedPtr<FEasyParticleRootState, ESPMode::ThreadSafe> RootPtr)
{
	if (RootPtr.IsValid())
	{
		ParticleRefArray.AddUnique(RootPtr);
	}
}

void FUIParticleFactory::ClearChildren(FEasyParticleRootState* RootPtr)
{
	for (int32 Layer = 0; Layer < ParticlePoolMapArray.Num(); Layer++)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePool = *pParticlePoolPtr;
			ParticlePoolMapArray[Layer].Remove(RootPtr);
			if (ParticlePool)
			{
				delete ParticlePool;
			}
		}
	}

}

void FUIParticleFactory::OnRootDestroy(FEasyParticleRootState* RootPtr)
{
	ClearChildren(RootPtr);
}

void FUIParticleFactory::AddReferencedMaterialInstances(FEasyParticleState* Emitter, FReferenceCollector& Collector)
{
	if (Emitter)
	{
		if (Cast<UMaterialInstanceDynamic>(Emitter->ParticleResource))
			Collector.AddReferencedObject(Emitter->ParticleResource);
		for (auto ChildrenArray : Emitter->ChildrenParticleArray)
		{
			for (auto ChildrenParticle : ChildrenArray.ChildrenParticle)
			{
				AddReferencedMaterialInstances(ChildrenParticle, Collector);
			}
		}
	}
}

void FUIParticleFactory::AddReferencedObjects(FReferenceCollector& Collector)
{
	for (int32 i = ParticleRefArray.Num()-1; i >= 0 ; i--)
	{
		if (ParticleRefArray[i].IsValid() && !ParticleRefArray[i].IsUnique())
		{
			if (ParticleRefArray[i]->RootAsset)
			{
				Collector.AddReferencedObject(ParticleRefArray[i]->RootAsset);
			}
			if (ParticleRefArray[i]->SelfAsset)
			{
				Collector.AddReferencedObject(ParticleRefArray[i]->SelfAsset);
			}
			AddReferencedMaterialInstances(ParticleRefArray[i].Get(), Collector);
		}
		else
		{
			ParticleRefArray.RemoveAt(i);
		}
	}
}

int32 FUIParticleFactory::GetChildrenCount(FEasyParticleRootState* RootPtr)
{
	int32 Count = 0;
	for (int32 Layer = 0; Layer < ParticlePoolMapArray.Num(); Layer++)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePoolPtr = *pParticlePoolPtr;
			if (ParticlePoolPtr)
			{
				Count = Count + ParticlePoolPtr->TotalCount - ParticlePoolPtr->FreeCount;
			}
		}
	}
	return Count;
}


class FTickParticleTask
{
	float DeltaTime;
	FGeometry AllottedGeometry;
	FEasyParticleStatePool* StatePool;
	int32 Index;
	float WholeScale;
	float RootSpanTime;
public:
	FTickParticleTask( float InDeltaTime, FGeometry InAllottedGeometry, FEasyParticleStatePool* InStatePool, int32 InWholeScale, int32 InRootSpanTime, int32 InIndex )
		: DeltaTime(InDeltaTime)
		, AllottedGeometry(InAllottedGeometry)
		, StatePool(InStatePool)
		, Index(InIndex)
		, WholeScale(InWholeScale)
		, RootSpanTime(InRootSpanTime)
	{
	}
	static FORCEINLINE TStatId GetStatId()
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FTickParticleTask, STATGROUP_ThreadPoolAsyncTasks);
	}
	static FORCEINLINE ENamedThreads::Type GetDesiredThread()
	{
		return ENamedThreads::AnyHiPriThreadHiPriTask;
	}
	static FORCEINLINE ESubsequentsMode::Type GetSubsequentsMode()
	{
		return ESubsequentsMode::TrackSubsequents;
	}
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		if (StatePool)
		{
			const FVector2D& LocalSize = AllottedGeometry.GetLocalSize();
			const FVector2D LocalPosition = LocalSize / 2;
			const FVector2D AbsolutePosition = AllottedGeometry.LocalToAbsolute(LocalPosition);
			FVector2D CurWidgetPosition = AbsolutePosition;

			const FSlateRenderTransform RenderTrans = AllottedGeometry.GetAccumulatedRenderTransform();
			float A, B, C, D;
			RenderTrans.GetMatrix().GetMatrix(A, B, C, D);
			float CurWidgetRotation = FMath::Atan2(B, A);
			
			int32 StartIndex = Index * COUNT_PARALLEL + MIN_COUNT_FOR_MULTITHREAD;
			int32 EndIndex = (Index + 1) * COUNT_PARALLEL + MIN_COUNT_FOR_MULTITHREAD;

			for (int32 i = StartIndex; i < EndIndex; i++)
			{
				FEasyParticleState* State = (*StatePool)[i];
				if (State)
				{
					State->TickSelf(DeltaTime, AllottedGeometry, CurWidgetPosition, CurWidgetRotation, WholeScale, RootSpanTime);
				}
			}
		}
	}
};

void FUIParticleFactory::TickRoot(FEasyParticleRootState* RootPtr, const float InDeltaTime, const FGeometry& AllottedGeometry)
{
	if (!UUIParticleUtility::GetMultiThread())
	{
		Tick(RootPtr, InDeltaTime, AllottedGeometry);
	}
	else
	{
		ParallelTick(RootPtr, InDeltaTime, AllottedGeometry);
	}
}

void FUIParticleFactory::ParallelTick(FEasyParticleRootState* RootPtr, const float InDeltaTime, const FGeometry& AllottedGeometry)
{
	if (RootPtr == nullptr)
		return;

	const FVector2D& LocalSize = AllottedGeometry.GetLocalSize();
	const FVector2D LocalPosition = LocalSize / 2;
	const FVector2D AbsolutePosition = AllottedGeometry.LocalToAbsolute(LocalPosition);
	FVector2D CurWidgetPosition = AbsolutePosition;

	const FSlateRenderTransform RenderTrans = AllottedGeometry.GetAccumulatedRenderTransform();
	float A, B, C, D;
	RenderTrans.GetMatrix().GetMatrix(A, B, C, D);
	float CurWidgetRotation = FMath::Atan2(B, A);

	RootPtr->TickSelf(InDeltaTime, AllottedGeometry, CurWidgetPosition, CurWidgetRotation);
	float WholeScale = RootPtr->WholeScale;
	float RootSpanTime = RootPtr->RootSpanTime;

	for (int32 Layer = 0; Layer < ParticlePoolMapArray.Num(); Layer++)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePoolPtr = *pParticlePoolPtr;
			if (ParticlePoolPtr)
			{
				int32 ChildrenCount = ParticlePoolPtr->Num();

				if (ChildrenCount <= MIN_COUNT_FOR_MULTITHREAD)
				{
					for (int32 i = 0; i < ChildrenCount; i++)
					{
						(*ParticlePoolPtr)[i]->TickSelf(InDeltaTime, AllottedGeometry, CurWidgetPosition, CurWidgetRotation, WholeScale, RootSpanTime);
					}
				}
				else
				{
					FGraphEventArray Tasks;
					int32 TaskCount = ((ChildrenCount - MIN_COUNT_FOR_MULTITHREAD) / COUNT_PARALLEL);
					if (((COUNT_PARALLEL * TaskCount) + MIN_COUNT_FOR_MULTITHREAD) < ChildrenCount)
					{
						TaskCount++;
					}
					Tasks.AddZeroed(TaskCount);

					ParallelFor(TaskCount,
						[&Tasks, InDeltaTime, &AllottedGeometry, ParticlePoolPtr, WholeScale, RootSpanTime](int32 Index)
					{
						Tasks[Index] = TGraphTask<FTickParticleTask>::CreateTask().ConstructAndDispatchWhenReady( InDeltaTime, AllottedGeometry, ParticlePoolPtr, WholeScale, RootSpanTime, Index);
					}
					);

					for (int32 i = 0; i < MIN_COUNT_FOR_MULTITHREAD; i++)
					{
						(*ParticlePoolPtr)[i]->TickSelf(InDeltaTime, AllottedGeometry, CurWidgetPosition, CurWidgetRotation, WholeScale, RootSpanTime);
					}
					FGraphEventRef Join = TGraphTask<FNullGraphTask>::CreateTask(&Tasks, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(TStatId(), ENamedThreads::AnyThread);
					FTaskGraphInterface::Get().WaitUntilTaskCompletes(Join, ENamedThreads::GameThread_Local);
				}
			}
		}
	}
}

void FUIParticleFactory::Tick(FEasyParticleRootState* RootPtr, const float InDeltaTime, const FGeometry& AllottedGeometry)
{
	if (RootPtr == nullptr)
		return;

	const FVector2D& LocalSize = AllottedGeometry.GetLocalSize();
	const FVector2D LocalPosition = LocalSize / 2;
	const FVector2D AbsolutePosition = AllottedGeometry.LocalToAbsolute(LocalPosition);
	FVector2D CurWidgetPosition = AbsolutePosition;

	const FSlateRenderTransform RenderTrans = AllottedGeometry.GetAccumulatedRenderTransform();
	float A, B, C, D;
	RenderTrans.GetMatrix().GetMatrix(A, B, C, D);
	float CurWidgetRotation = FMath::Atan2(B, A);

	RootPtr->TickSelf(InDeltaTime, AllottedGeometry, CurWidgetPosition, CurWidgetRotation);
	float WholeScale = RootPtr->WholeScale;
	float RootSpanTime = RootPtr->RootSpanTime;

	for (int32 Layer = 0; Layer < ParticlePoolMapArray.Num(); Layer++)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePoolPtr = *pParticlePoolPtr;
			if (ParticlePoolPtr)
			{
				int32 ChildrenCount = ParticlePoolPtr->Num();
				for (int32 i = 0; i < ChildrenCount; i++)
				{
					(*ParticlePoolPtr)[i]->TickSelf(InDeltaTime, AllottedGeometry, CurWidgetPosition, CurWidgetRotation, WholeScale, RootSpanTime);
				}
			}
		}
	}
}



class FPaintParticleTask
{
	const FPaintArgs& Args;
	FGeometry AllottedGeometry;
	const FWidgetStyle& WidgetStyle;
	FEasyParticleStatePool* StatePool;
	int32 Index;
public:
	FPaintParticleTask(const FPaintArgs& InArgs, FGeometry InAllottedGeometry, const FWidgetStyle& InWidgetStyle, FEasyParticleStatePool* InStatePool, int32 InIndex)
		: Args(InArgs)
		, AllottedGeometry(InAllottedGeometry)
		, WidgetStyle(InWidgetStyle)
		, StatePool(InStatePool)
		, Index(InIndex)
	{
	}
	static FORCEINLINE TStatId GetStatId()
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FTickParticleTask, STATGROUP_ThreadPoolAsyncTasks);
	}
	static FORCEINLINE ENamedThreads::Type GetDesiredThread()
	{
		return ENamedThreads::AnyHiPriThreadHiPriTask;
	}
	static FORCEINLINE ESubsequentsMode::Type GetSubsequentsMode()
	{
		return ESubsequentsMode::TrackSubsequents;
	}
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		if (StatePool)
		{
			int32 StartIndex = Index * COUNT_PARALLEL_PAINT + MIN_COUNT_FOR_MULTITHREAD_PAINT;
			int32 EndIndex = (Index + 1) * COUNT_PARALLEL_PAINT + MIN_COUNT_FOR_MULTITHREAD_PAINT;

			for (int32 i = StartIndex; i < EndIndex; i++)
			{
				FEasyParticleState* State = (*StatePool)[i];
				if (State)
				{
					State->CaculatePaintParams(Args, AllottedGeometry, WidgetStyle);
				}
			}
		}
	}
};



void FUIParticleFactory::OnRootPaint(FEasyParticleRootState* RootPtr, const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled)
{
	SCOPE_CYCLE_COUNTER(STAT_UIParticleOnPaintTime);
	if (!UUIParticleUtility::GetMultiThread())
	{
		OnPaint(RootPtr, Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}
	else
	{
		OnParallelPaint(RootPtr, Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}
	/*
	RootPtr->OnPaintSelf(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	for (int32 Layer = 0; Layer < ParticlePoolMapArray.Num(); Layer++)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePoolPtr = *pParticlePoolPtr;
			if (ParticlePoolPtr)
			{
				int32 ChildrenCount = ParticlePoolPtr->Num();
				for (int32 i = 0; i < ChildrenCount; i++)
				{
					(*ParticlePoolPtr)[i]->OnPaintSelf(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
				}
			}
		}
	}
	*/
	RootPtr->OnPaint( Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

void FUIParticleFactory::OnPaint(FEasyParticleRootState* RootPtr, const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled)
{
	if (RootPtr == nullptr)
		return;

	RootPtr->CaculatePaintParams(Args, AllottedGeometry,InWidgetStyle);
	for (int32 Layer = 0; Layer < ParticlePoolMapArray.Num(); Layer++)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePoolPtr = *pParticlePoolPtr;
			if (ParticlePoolPtr)
			{
				int32 ChildrenCount = ParticlePoolPtr->Num();
				for (int32 i = 0; i < ChildrenCount; i++)
				{
					(*ParticlePoolPtr)[i]->CaculatePaintParams(Args, AllottedGeometry,InWidgetStyle);
				}
			}
		}
	}
}

void FUIParticleFactory::OnParallelPaint(FEasyParticleRootState* RootPtr, const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled)
{
	if (RootPtr == nullptr)
		return;

	RootPtr->CaculatePaintParams(Args, AllottedGeometry , InWidgetStyle);

	for (int32 Layer = 0; Layer < ParticlePoolMapArray.Num(); Layer++)
	{
		FEasyParticleStatePool** pParticlePoolPtr = ParticlePoolMapArray[Layer].Find(RootPtr);
		if (pParticlePoolPtr)
		{
			FEasyParticleStatePool* ParticlePoolPtr = *pParticlePoolPtr;
			if (ParticlePoolPtr)
			{
				int32 ChildrenCount = ParticlePoolPtr->Num();

				if (ChildrenCount <= MIN_COUNT_FOR_MULTITHREAD_PAINT)
				{
					for (int32 i = 0; i < ChildrenCount; i++)
					{
						(*ParticlePoolPtr)[i]->CaculatePaintParams(Args, AllottedGeometry,InWidgetStyle);
					}
				}
				else
				{
					FGraphEventArray Tasks;
					int32 TaskCount = ((ChildrenCount - MIN_COUNT_FOR_MULTITHREAD_PAINT) / COUNT_PARALLEL_PAINT);
					if (((COUNT_PARALLEL_PAINT * TaskCount) + MIN_COUNT_FOR_MULTITHREAD_PAINT) < ChildrenCount)
					{
						TaskCount++;
					}
					Tasks.AddZeroed(TaskCount);

					ParallelFor(TaskCount,
						[&Tasks, &Args, &AllottedGeometry,&InWidgetStyle, ParticlePoolPtr](int32 Index)
					{
						Tasks[Index] = TGraphTask<FPaintParticleTask>::CreateTask().ConstructAndDispatchWhenReady(Args, AllottedGeometry, InWidgetStyle, ParticlePoolPtr, Index);
					}
					);

					for (int32 i = 0; i < MIN_COUNT_FOR_MULTITHREAD_PAINT; i++)
					{
						(*ParticlePoolPtr)[i]->CaculatePaintParams(Args, AllottedGeometry, InWidgetStyle);
					}
					FGraphEventRef Join = TGraphTask<FNullGraphTask>::CreateTask(&Tasks, ENamedThreads::GameThread).ConstructAndDispatchWhenReady(TStatId(), ENamedThreads::AnyThread);
					FTaskGraphInterface::Get().WaitUntilTaskCompletes(Join, ENamedThreads::GameThread_Local);
				}
			}
		}
	}
}