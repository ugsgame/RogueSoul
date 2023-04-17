// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleEditorModule.h"
#include "UIParticleEditorPrivatePCH.h"
/*
#include "SConstraintCanvas.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "LevelEditor.h"
*/
#include "UIParticleEditorStyle.h"
#include "UIParticleEditorCommands.h"
#include "UIParticleAssetTypeActions.h"
#include "UIParticleLeafAssetTypeActions.h"
#include "Asset/UIParticleEmitterAsset.h"
#include "Widget/UIParticle.h"
#include "Widget/UIParticleEmitter.h"
#include "UIParticleCustomization.h"
#include "UIParticleEmitterCustomization.h"
#include "UIParticleLinearColorCurveCustomization.h"
#include "UIParticleFloatCurveCustomization.h"
#include "UIParticlePropertyCustomization.h"

static const FName UIParticleEditorTabName("UIParticleEditor");

#define LOCTEXT_NAMESPACE "FUIParticleEditorModule"

void FUIParticleEditorModule::StartupModule()
{
	FUIParticleEditorStyle::Initialize();
	FUIParticleEditorStyle::ReloadTextures();

	FUIParticleEditorCommands::Register();
	FUIParticleEmitterEditorCommands::Register();

	IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
    RegisterAssetTypeAction(AssetTools, MakeShareable(new FUIParticleAssetTypeActions()));
    RegisterAssetTypeAction(AssetTools, MakeShareable(new FUIParticleLeafAssetTypeActions()));
	
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		PropertyModule.RegisterCustomClassLayout(UUIParticle::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIParticleCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(UUIParticleEmitter::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FUIParticleEmitterCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout("UIParticleLinearColorCurve", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUIParticleLinearColorCurveCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout("UIParticleFloatCurve", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUIParticleFloatCurveCustomization::MakeInstance));
		PropertyModule.RegisterCustomPropertyTypeLayout("UIParticleProperty", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUIParticlePropertyCustomization::MakeInstance));
	}
}

void FUIParticleEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FUIParticleEditorStyle::Shutdown();

	FUIParticleEditorCommands::Unregister();
	FUIParticleEmitterEditorCommands::Unregister();
	
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UIParticleEditorTabName);

	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();
	
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		PropertyModule.UnregisterCustomClassLayout(UUIParticle::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UUIParticleEmitter::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomPropertyTypeLayout("UIParticleLinearColorCurve");
		PropertyModule.UnregisterCustomPropertyTypeLayout("UIParticleFloatCurve");
		PropertyModule.UnregisterCustomPropertyTypeLayout("UIParticleProperty");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FUIParticleEditorModule, UIParticleEditor)
