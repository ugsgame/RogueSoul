// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleAssetFactory.h"
#include "UIParticleEditorPrivatePCH.h"
#include "Asset/UIParticleAsset.h"

#define LOCTEXT_NAMESPACE "UUMG_Widget_Brush"


UUIParticleAssetFactory::UUIParticleAssetFactory(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UUIParticleAsset::StaticClass();
}

FText UUIParticleAssetFactory::GetDisplayName() const
{
	return LOCTEXT("UIParticleAssetFactoryDescription", "UIParticle");
}

bool UUIParticleAssetFactory::ConfigureProperties()
{
	return true;
}

UObject* UUIParticleAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return CastChecked<UUIParticleAsset>(NewObject<UUIParticleAsset>(InParent, Name, Flags));
}

#undef LOCTEXT_NAMESPACE
