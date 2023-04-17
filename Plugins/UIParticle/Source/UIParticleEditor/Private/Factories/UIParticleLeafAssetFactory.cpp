// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleLeafAssetFactory.h"
#include "UIParticleLeafAssetTypeActions.h"
#include "UIParticleEditorPrivatePCH.h"

#define LOCTEXT_NAMESPACE "UUMG_Widget_Brush"


UUIParticleLeafAssetFactory::UUIParticleLeafAssetFactory(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UUIParticleEmitterAsset::StaticClass();
}

FText UUIParticleLeafAssetFactory::GetDisplayName() const
{
	return LOCTEXT("UIParticleDataFactoryDescription", "UIParticleEmitter");
}

bool UUIParticleLeafAssetFactory::ConfigureProperties()
{
	return true;
}

UObject* UUIParticleLeafAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return CastChecked<UUIParticleEmitterAsset>(NewObject<UUIParticleEmitterAsset>(InParent, Name, Flags));
}

#undef LOCTEXT_NAMESPACE
