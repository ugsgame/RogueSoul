// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleModule.h"
#include "Modules/ModuleManager.h"
#include "Utility/UIParticleUtility.h"
#include "UIParticlePrivatePCH.h"
//#include "SlateBasics.h"
//#include "SlateExtras.h"

#define LOCTEXT_NAMESPACE "FUIParticleModule"
DEFINE_LOG_CATEGORY_STATIC(LogUIParticle, All, All);

void FUIParticleModule::StartupModule()
{
	SetLODCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("uiparticle.SetUIParticleLOD"),
		TEXT("Set UIParticle's LOD"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUIParticleModule::SetUIParticleLOD));
	GetLODCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("uiparticle.GetUIParticleLOD"),
		TEXT("Print UIParticle's LOD"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUIParticleModule::GetUIParticleLOD));


	SetMultiThreadCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("uiparticle.SetMultiThread"),
		TEXT("Set UIParticle's MultiThread"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUIParticleModule::SetUIParticleMultiThread));
	GetMultiThreadCommand = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("uiparticle.GetMultiThread"),
		TEXT("Print UIParticle's MultiThread"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FUIParticleModule::GetUIParticleMultiThread));

}

void FUIParticleModule::ShutdownModule()
{
	if (SetLODCommand != nullptr)
	{
		IConsoleManager::Get().UnregisterConsoleObject(SetLODCommand);
		SetLODCommand = nullptr;
	}
	if (GetLODCommand != nullptr)
	{
		IConsoleManager::Get().UnregisterConsoleObject(GetLODCommand);
		GetLODCommand = nullptr;
	}
	if (SetMultiThreadCommand != nullptr)
	{
		IConsoleManager::Get().UnregisterConsoleObject(SetMultiThreadCommand);
		SetMultiThreadCommand = nullptr;
	}
	if (GetMultiThreadCommand != nullptr)
	{
		IConsoleManager::Get().UnregisterConsoleObject(GetMultiThreadCommand);
		GetMultiThreadCommand = nullptr;
	}
}

void FUIParticleModule::SetUIParticleLOD(const TArray<FString>& Arguments)
{
	if (Arguments.Num() == 1)
	{
		FString LodStr = Arguments[0];
		int32 lod = 0;
		LexFromString(lod, *LodStr);
		UUIParticleUtility::SetLOD(lod);
	}
	else
	{
		UE_LOG(LogUIParticle, Error, TEXT("Param Level was missing"));
	}
}

void FUIParticleModule::GetUIParticleLOD(const TArray<FString>& Arguments)
{
	int32 lod = UUIParticleUtility::GetLOD();
	UE_LOG(LogUIParticle, Display, TEXT("UIParticle LOD is %d"),lod);
}

void FUIParticleModule::SetUIParticleMultiThread(const TArray<FString>& Arguments)
{
	if (Arguments.Num() == 1)
	{
		FString MultiThreadStr = Arguments[0];
		int32 value = 0;
		LexFromString(value, *MultiThreadStr);
		UUIParticleUtility::SetMultiThread(value == 1);
	}
	else
	{
		UE_LOG(LogUIParticle, Error, TEXT("Param Value was missing"));
	}
}

void FUIParticleModule::GetUIParticleMultiThread(const TArray<FString>& Arguments)
{
	int32 value = UUIParticleUtility::GetMultiThread();
	UE_LOG(LogUIParticle, Display, TEXT("UIParticle MultiThread is %d"), value);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUIParticleModule, UIParticle)
