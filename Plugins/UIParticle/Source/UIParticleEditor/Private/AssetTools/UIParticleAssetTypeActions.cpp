// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleAssetTypeActions.h"
#include "UIParticleEditorPrivatePCH.h"
#include "UIParticleEditorModule.h"
#include "Asset/UIParticleAsset.h"
#include "ToolKits/UIParticleEditorToolKit.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FUIParticleAssetTypeActions::FUIParticleAssetTypeActions()
{
}

FText FUIParticleAssetTypeActions::GetName() const
{
	return LOCTEXT("FUIParticleAssetTypeActionsName", "UIParticleAsset");
}

FColor FUIParticleAssetTypeActions::GetTypeColor() const
{
	return FColor(255, 0, 0);
}

UClass* FUIParticleAssetTypeActions::GetSupportedClass() const
{
	return UUIParticleAsset::StaticClass();
}

void FUIParticleAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UUIParticleAsset* ParticleAsset = Cast<UUIParticleAsset>(*ObjIt))
		{
			TSharedRef<FUIParticleEditorToolKit> NewUIParticleEditor(new FUIParticleEditorToolKit());
			NewUIParticleEditor->InitUIParticleEditor(Mode, EditWithinLevelEditor,ParticleAsset);
		}
	}
}

uint32 FUIParticleAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::UI ;
}


/*
TSharedPtr<class SWidget> FUIParticleAssetTypeActions::GetThumbnailOverlay(const FAssetData& AssetData) const
{
	TSharedPtr<SUIParticleEmitter> Thumbnail = SNew(SUIParticleEmitter);
	UUIParticleEmitterAsset* asset = (UUIParticleEmitterAsset*)AssetData.GetAsset();
	Thumbnail->InitWithAsset(asset);
	Thumbnail->BeginParticle();

	return SNew(SConstraintCanvas)
		+ SConstraintCanvas::Slot()
		.Anchors(FAnchors(0, 0, 1.0, 1.0))
		.Offset(FMargin(0.0, 0.0, 0.0, 0))
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor(FLinearColor::Black))
		]
	+ SConstraintCanvas::Slot()
		.Anchors(FAnchors(0, 0, 1.0, 1.0))
		.Offset(FMargin(0.0, 0.0, 0, 0))
		[
			Thumbnail.ToSharedRef()
		];
}*/

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
