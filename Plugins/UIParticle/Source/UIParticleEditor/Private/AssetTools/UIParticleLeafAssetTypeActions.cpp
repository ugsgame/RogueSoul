// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleLeafAssetTypeActions.h"
#include "UIParticleLeafAssetFactory.h"
#include "UIParticleEditorPrivatePCH.h"
#include "UIParticleEditorModule.h"
#include "Asset/UIParticleAsset.h"
#include "ToolKits/UIParticleEmitterToolKit.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FUIParticleLeafAssetTypeActions::FUIParticleLeafAssetTypeActions()
{
}

FText FUIParticleLeafAssetTypeActions::GetName() const
{
	return LOCTEXT("FUIParticleLeafAssetTypeActionsName", "UIParticleEmitter");
}

FColor FUIParticleLeafAssetTypeActions::GetTypeColor() const
{
	return FColor(129, 6, 5);
}

UClass* FUIParticleLeafAssetTypeActions::GetSupportedClass() const
{
	return UUIParticleEmitterAsset::StaticClass();
}

uint32 FUIParticleLeafAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::UI ;
}


void FUIParticleLeafAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
    const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
    
    for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
    {
        if (UUIParticleEmitterAsset* ParticleAsset = Cast<UUIParticleEmitterAsset>(*ObjIt))
        {
            TSharedRef<FUIParticleEmitterToolKit> NewUIParticleEditor(new FUIParticleEmitterToolKit());
            NewUIParticleEditor->InitUIParticleEditor(Mode, EditWithinLevelEditor,ParticleAsset);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
