// Copyright 2018 Francesco Desogus. All Rights Reserved.

#include "TweenManagerActor.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "TweenManagerComponent.h"
#include "TweenContainer.h"
#include "Utils/Utility.h"


UTweenManagerComponent* ATweenManagerActor::Instance = nullptr;

ATweenManagerActor::ATweenManagerActor()
{
	PrimaryActorTick.bCanEverTick = true;
}


void ATweenManagerActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	const UWorld* world = GetWorld();
	
	if(world && world->IsGameWorld())
	{
		// Instancing the singleton and registering with this Actor
		if(Instance == nullptr)
		{
			Instance = NewObject<UTweenManagerComponent>(this);
			Instance->RegisterComponent();
		}
		else
		{
			UE_LOG(LogTweenMaker, Warning, TEXT("ATweenManagerActor::PostInitializeComponents() -> there should be only ONE TweenManagerActor in the level!"));
		}
	}
}

void ATweenManagerActor::BeginPlay()
{
	Super::BeginPlay();
	SetTickableWhenPaused(true);
}

TWEENMAKER_API UTweenManagerComponent* ATweenManagerActor::GetInstance(const UObject* WorldContent/* = nullptr*/)
{
	if (Instance == nullptr)
	{
		if (GWorld)
		{
			GWorld->SpawnActor<ATweenManagerActor>();
		}
	}
	if (Instance == nullptr)
	{
		if (WorldContent && WorldContent->GetWorld())
		{
			WorldContent->GetWorld()->SpawnActor<ATweenManagerActor>();
		}
	}
	if (Instance == nullptr)
	{
		UE_LOG(LogTweenMaker, Error, TEXT("ATweenManagerActor::GetInstance() -> couldn't find the instance of ATweenManagerActor in the level. "
			"If you want to use Tweens with a global manager, add a ATweenManagerActor in your level."));
	}
	return Instance;
}

void ATweenManagerActor::BP_CreateTweenContainerStatic(UTweenContainer *&pOutTweenContainer, int32 pNumLoops, ETweenLoopType pLoopType, float pTimeScale)
{
    pOutTweenContainer = nullptr;

    UTweenManagerComponent *instance = GetInstance();

    if(instance != nullptr)
    {
        pOutTweenContainer = instance->CreateTweenContainer(pNumLoops, pLoopType, pTimeScale);
    }
}

int32 ATweenManagerActor::DeleteAllTweens()
{
    int32 tweensDeleted = 0;
    UTweenManagerComponent *instance = GetInstance();

    if(instance != nullptr)
    {
        tweensDeleted = instance->DeleteAllTweens();
    }

    return tweensDeleted;
}


bool ATweenManagerActor::DeleteAllTweensByObject(UObject *pTweenTarget, ETweenGenericType pTweensType)
{
    bool hasDeletedSomething = false;
    UTweenManagerComponent *instance = GetInstance();

    if(instance != nullptr)
    {
       hasDeletedSomething = instance->DeleteAllTweensByObject(pTweenTarget, pTweensType);
    }

    return hasDeletedSomething;
}

bool ATweenManagerActor::IsObjectTweening(UObject *pTweenTarget, ETweenGenericType pTweensType, UBaseTween *&pTweenFound)
{
    bool isObjectTweening = false;
    UTweenManagerComponent *instance = GetInstance();
    pTweenFound = nullptr;

    if(instance != nullptr)
    {
       isObjectTweening = instance->IsObjectTweening(pTweenTarget, pTweensType, pTweenFound);
    }

    return isObjectTweening;
}

bool ATweenManagerActor::FindTweenByName(FName pTweenName, ETweenGenericType pTweenType, UBaseTween *&pTweenFound)
{
    bool tweenFound = false;
    UTweenManagerComponent *instance = GetInstance();
    pTweenFound = nullptr;

    if(instance != nullptr)
    {
       tweenFound = instance->FindTweenByName(pTweenName, pTweenType, pTweenFound);
    }

    return tweenFound;
}

void ATweenManagerActor::BeginDestroy()
{
    Super::BeginDestroy();

    Instance = nullptr;
}

void ATweenManagerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

