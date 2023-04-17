// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Modules/ModuleInterface.h"
#include "HAL/IConsoleManager.h"

class FUIParticleModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void SetUIParticleLOD(const TArray<FString>& Arguments);
	void GetUIParticleLOD(const TArray<FString>& Arguments);
	void SetUIParticleMultiThread(const TArray<FString>& Arguments);
	void GetUIParticleMultiThread(const TArray<FString>& Arguments);
	IConsoleCommand* SetLODCommand;
	IConsoleCommand* GetLODCommand;
	IConsoleCommand* SetMultiThreadCommand;
	IConsoleCommand* GetMultiThreadCommand;
};
