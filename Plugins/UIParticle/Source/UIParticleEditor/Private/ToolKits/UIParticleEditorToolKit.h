// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Misc/NotifyHook.h"
#include "UObject/GCObject.h"
#include "Asset/UIParticleAsset.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Docking/TabManager.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Slate/WidgetRenderer.h"
#include "Editor/EditorWidgets/Public/ITransportControl.h"


class UUIParticleAsset;
class SUIParticle;
class SUIParticleEmittersList;

//////////////////////////////////////////////////////////////////////////
// FUIParticleEditorToolKit

class FUIParticleEditorToolKit : public FAssetEditorToolkit, public FGCObject ,public FNotifyHook
{

public:
	FUIParticleEditorToolKit();

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	// End of IToolkit interface


	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	//virtual FString GetDocumentationLink() const override;
	// End of FAssetEditorToolkit

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

    //~ Begin FNotifyHook Interface
    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
    //~ End FNotifyHook Interface
    
protected:
	void BindCommands();
	void ExtendMenu();
	void ExtendToolbar();

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_EmittersList(const FSpawnTabArgs& Args);
    
public:

	void BeginPlay();
	void Stop();
	void StopEmit();
	void CaptureThumbnail();
	void ClearThumbnail();
	TSharedPtr<SUIParticle> ParticlePanel;
    TSharedPtr<STextBlock> InfoText;
    TSharedPtr<IDetailsView> EmitterList;
	FText GetCurrentParticleCountText() const;
	TSharedPtr<FWidgetRenderer> WidgetRenderer;
public:
	void InitUIParticleEditor(const EToolkitMode::Type Mode, TSharedPtr<class IToolkitHost> InitToolkitHost , UUIParticleAsset* ObjectToEdit);

	/** Need Timer to tick the widget */
	TSharedPtr<FActiveTimerHandle> UpdatePluginsWidget;
	EActiveTimerReturnType UpdateWidgetFunc(double InCurrentTime, float InDeltaTime);
protected:
	UUIParticleAsset* AssetBeingEdited;
    
};
