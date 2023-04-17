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
#include "Editor/EditorWidgets/Public/ITransportControl.h"

//thumbnail need
#include "Misc/ObjectThumbnail.h"
#include "ObjectTools.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
//thumbnail need

class UUIParticleAsset;
class SUIParticleEmitter;
class SUIParticleEmittersList;

//////////////////////////////////////////////////////////////////////////
// FUIParticleEmitterToolKit

class FUIParticleEmitterToolKit : public FAssetEditorToolkit, public FGCObject ,public FNotifyHook
{

public:
	FUIParticleEmitterToolKit();


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
    TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
    
public:

	void BeginPlay();
	void Stop();
	void StopEmit();
	void CaptureThumbnail();
	void ClearThumbnail();
	TSharedPtr<SUIParticleEmitter> ParticlePanel;
	TSharedPtr<STextBlock> InfoText;
    TSharedPtr<IDetailsView> DetailsViewPanel;
	FText GetCurrentParticleCountText() const;
	TSharedPtr<FWidgetRenderer> WidgetRenderer;
public:
	void InitUIParticleEditor(const EToolkitMode::Type Mode, TSharedPtr<class IToolkitHost> InitToolkitHost , UUIParticleEmitterAsset* ObjectToEdit);


	/** Need Timer to tick the widget */
	TSharedPtr<FActiveTimerHandle> UpdatePluginsWidget;
	EActiveTimerReturnType UpdateWidgetFunc(double InCurrentTime, float InDeltaTime);
protected:
	UUIParticleEmitterAsset* AssetBeingEdited;
};

class STextBlockChild : public STextBlock
{
public:
	TSharedRef<FActiveTimerHandle> RegisterTickActiveTimer(FWidgetActiveTimerDelegate TickFunction)
	{
		return RegisterActiveTimer(0.f, TickFunction);
	};

	void UnRegisterTickActiveTimer(const TSharedRef<FActiveTimerHandle>& ActiveTimerHandle)
	{
		UnRegisterActiveTimer(ActiveTimerHandle);
	}
};


static bool GetRawData(UTextureRenderTarget2D* TexRT, TArray<uint8>& RawData)
{
	FRenderTarget* RenderTarget = TexRT->GameThread_GetRenderTargetResource();
	EPixelFormat Format = TexRT->GetFormat();

	int32 ImageBytes = CalculateImageBytes(TexRT->SizeX, TexRT->SizeY, 0, Format);
	RawData.AddUninitialized(ImageBytes);
	bool bReadSuccess = false;
	switch (Format)
	{
	case PF_FloatRGBA:
	{
		TArray<FFloat16Color> FloatColors;
		bReadSuccess = RenderTarget->ReadFloat16Pixels(FloatColors);
		FMemory::Memcpy(RawData.GetData(), FloatColors.GetData(), ImageBytes);
	}
	break;
	case PF_B8G8R8A8:
		bReadSuccess = RenderTarget->ReadPixelsPtr((FColor*)RawData.GetData());
		break;
	}
	if (bReadSuccess == false)
	{
		RawData.Empty();
	}
	return bReadSuccess;
};
