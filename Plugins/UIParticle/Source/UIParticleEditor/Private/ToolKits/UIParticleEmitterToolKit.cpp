// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleEmitterToolKit.h"
//#include "UIParticleEditorToolKit.h"
#include "UIParticleEditorPrivatePCH.h"
#include "Widget/SUIParticleEmitter.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"
#include "PropertyEditorModule.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "UIParticleEditorCommands.h"
//#include "EditorViewportClient.h"
//#include "UObject/Package.h"
//#include "ScopedTransaction.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SConstraintCanvas.h"

#define LOCTEXT_NAMESPACE "UIParticleEmitterEditor"

const FName FUIParticleEmitterAppName = FName(TEXT("UIParticleEmitterEditorApp"));

struct FUIParticleEmitterTabs
{
	// Tab identifiers
	static const FName DetailsID;
    static const FName ViewportID;
};

const FName FUIParticleEmitterTabs::DetailsID(TEXT("Details"));
const FName FUIParticleEmitterTabs::ViewportID(TEXT("Viewport"));

FUIParticleEmitterToolKit::FUIParticleEmitterToolKit()
	: AssetBeingEdited(nullptr)
{
}

TSharedRef<SDockTab> FUIParticleEmitterToolKit::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	ParticlePanel = SNew(SUIParticleEmitter);
	TSharedPtr<STextBlockChild> InfoTextTemp = SNew(STextBlockChild)
		.ColorAndOpacity(FLinearColor(0, 1, 0, 1))
		.Text(this, &FUIParticleEmitterToolKit::GetCurrentParticleCountText);

	if (UpdatePluginsWidget.IsValid())
	{
		InfoTextTemp->UnRegisterTickActiveTimer(UpdatePluginsWidget.ToSharedRef());
	}
	UpdatePluginsWidget = InfoTextTemp->RegisterTickActiveTimer( FWidgetActiveTimerDelegate::CreateSP(this, &FUIParticleEmitterToolKit::UpdateWidgetFunc));

	InfoText = InfoTextTemp;
	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			SNew(SConstraintCanvas)
			+ SConstraintCanvas::Slot()
				.Offset(FMargin(0.0, 0.0, 0.0, 0.0))
				.Anchors(FAnchors(0.0, 0.0, 1.0, 1.0))
				[
					SNew(SImage)
					.ColorAndOpacity(FLinearColor(0, 0, 0, 1))
				]
			+ SConstraintCanvas::Slot()
				.Offset(FMargin(0.0, 0.0 , 0.0, 0.0))
				.Anchors(FAnchors(1.0, 0.0, 1.0, 0.0))
				.Alignment(FVector2D(1.0, 0.0))
				.AutoSize(true)
				[
					InfoText.ToSharedRef()
				]
             + SConstraintCanvas::Slot()
				.Anchors(FAnchors(0, 0, 1.0, 1.0))
				.Offset(FMargin(0.0, 0.0, 0.0, 0))
				[
					ParticlePanel.ToSharedRef()
				]
		];
}

TSharedRef<SDockTab> FUIParticleEmitterToolKit::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    
    DetailsViewPanel->SetObject(AssetBeingEdited);
    // Spawn the tab
	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			DetailsViewPanel.ToSharedRef()
		];
    
}

void FUIParticleEmitterToolKit::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	FString PropertyName = PropertyThatChanged->GetName();
	if (PropertyName != "ScalarParams" && PropertyName != "ChildrenEmitters" && PropertyName != "ScalarParamsWhenStart" && PropertyName != "LevelOfDetail")
	{
		this->BeginPlay();
	}
	if (PropertyName == "LevelOfDetail")
	{
		this->Stop();
	}
}
FName FUIParticleEmitterToolKit::GetToolkitFName() const
{
	return FName("UIParticleEmitterEditor");
}

FText FUIParticleEmitterToolKit::GetBaseToolkitName() const
{
	return LOCTEXT("UIParticleEmitterEditorAppLabel", "UIParticleEmitterEditor");
}

FText FUIParticleEmitterToolKit::GetToolkitName() const
{
	const bool bDirtyState = AssetBeingEdited->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("UIParticleName"), FText::FromString(AssetBeingEdited->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("UIParticleEditorToolkitName", "{UIParticleName}{DirtyState}"), Args);
}


FText FUIParticleEmitterToolKit::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(AssetBeingEdited);
}


FLinearColor FUIParticleEmitterToolKit::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

void FUIParticleEmitterToolKit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(AssetBeingEdited);
}

FString FUIParticleEmitterToolKit::GetWorldCentricTabPrefix() const
{
	return TEXT("UIParticleEmitterEditor");
}


void FUIParticleEmitterToolKit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_UIParticleEditor", "UIParticle Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FUIParticleEmitterTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FUIParticleEmitterToolKit::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FUIParticleEmitterTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FUIParticleEmitterToolKit::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
    
}

void FUIParticleEmitterToolKit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FUIParticleEmitterTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FUIParticleEmitterTabs::DetailsID);
}


void FUIParticleEmitterToolKit::InitUIParticleEditor(const EToolkitMode::Type Mode, TSharedPtr<class IToolkitHost> InitToolkitHost, UUIParticleEmitterAsset* ObjectToEdit)
{
	// Initialize the asset editor and spawn nothing (dummy layout)
	AssetBeingEdited = ObjectToEdit;
	BindCommands();
	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_UIParticleEmitterEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->SetHideTabWell(true)
					->AddTab(FUIParticleEmitterTabs::ViewportID, ETabState::OpenedTab)
                )
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(FUIParticleEmitterTabs::DetailsID, ETabState::OpenedTab)
				)
			)
		);
	InitAssetEditor(Mode, InitToolkitHost, FUIParticleEditorAppName, StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, ObjectToEdit);
	

	// Extend things
	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();
}


FText FUIParticleEmitterToolKit::GetCurrentParticleCountText() const
{
	if (AssetBeingEdited && ParticlePanel.IsValid())
	{
		return FText::Format(LOCTEXT("ParticleCount", "ParticleCount: {0}  Pool: {1}"), FText::AsNumber(ParticlePanel->GetParticleCount()), FText::AsNumber(ParticlePanel->GetParticleCountInPool()));
	}

	return FText::GetEmpty();
}



void FUIParticleEmitterToolKit::BindCommands()
{
	const FUIParticleEmitterEditorCommands& Commands = FUIParticleEmitterEditorCommands::Get();
	const TSharedRef<FUICommandList>& UICommandList = GetToolkitCommands();
	UICommandList->MapAction(Commands.BeginPlay_Emitter, FExecuteAction::CreateSP(this, &FUIParticleEmitterToolKit::BeginPlay));
	UICommandList->MapAction(Commands.Stop_Emitter, FExecuteAction::CreateSP(this, &FUIParticleEmitterToolKit::Stop));
	UICommandList->MapAction(Commands.StopEmit_Emitter, FExecuteAction::CreateSP(this, &FUIParticleEmitterToolKit::StopEmit));
	UICommandList->MapAction(Commands.CaptureThumbnail_Emitter, FExecuteAction::CreateSP(this, &FUIParticleEmitterToolKit::CaptureThumbnail));
	UICommandList->MapAction(Commands.ClearThumbnail_Emitter, FExecuteAction::CreateSP(this, &FUIParticleEmitterToolKit::ClearThumbnail));
}
void FUIParticleEmitterToolKit::ExtendMenu()
{
}

void FUIParticleEmitterToolKit::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("Command");
			{
				ToolbarBuilder.AddToolBarButton(FUIParticleEmitterEditorCommands::Get().BeginPlay_Emitter, NAME_None, LOCTEXT("PlayParticle", "Play"), LOCTEXT("PlayParticleTooltip", "Play ParticleEmitter"), FSlateIcon(FUIParticleEditorStyle::GetStyleSetName(), "ParticleAsset.AssetActions.Play.Large"));
				ToolbarBuilder.AddToolBarButton(FUIParticleEmitterEditorCommands::Get().Stop_Emitter, NAME_None, LOCTEXT("StopParticle", "Stop"), LOCTEXT("StopParticleTooltip", "Stop ParticleEmitter"), FSlateIcon(FUIParticleEditorStyle::GetStyleSetName(), "ParticleAsset.AssetActions.Stop.Large"));
				ToolbarBuilder.AddToolBarButton(FUIParticleEmitterEditorCommands::Get().StopEmit_Emitter, NAME_None, LOCTEXT("StopEmitParticle", "StopEmit"), LOCTEXT("StopEmitParticleTooltip", "StopEmit ParticleEmitter"), FSlateIcon(FUIParticleEditorStyle::GetStyleSetName(), "ParticleAsset.AssetActions.Stop.Large"));
				ToolbarBuilder.AddToolBarButton(FUIParticleEmitterEditorCommands::Get().CaptureThumbnail_Emitter, NAME_None, LOCTEXT("CaptureThumbnail", "CaptureThumbnail"), LOCTEXT("CaptureThumbnail_Tooltip", "Capture Thumbnail"), FSlateIcon(FUIParticleEditorStyle::GetStyleSetName(), "ParticleAsset.AssetActions.CaptureThumbnail.Large"));
				ToolbarBuilder.AddToolBarButton(FUIParticleEmitterEditorCommands::Get().ClearThumbnail_Emitter, NAME_None, LOCTEXT("ClearThumbnail", "ClearThumbnail"), LOCTEXT("ClearThumbnail_Tooltip", "Clear Thumbnail"), FSlateIcon(FUIParticleEditorStyle::GetStyleSetName(), "ParticleAsset.AssetActions.ClearThumbnail.Large"));
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		/*ViewportPtr->GetCommandList()*/ GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
	);

	AddToolbarExtender(ToolbarExtender);
}


void FUIParticleEmitterToolKit::BeginPlay()
{
	if (AssetBeingEdited && AssetBeingEdited->IsValidLowLevel() && ParticlePanel.IsValid())
	{
		ParticlePanel->InitWithAsset(AssetBeingEdited);
		ParticlePanel->Play();
	}
}
void FUIParticleEmitterToolKit::Stop()
{
	if (AssetBeingEdited && ParticlePanel.IsValid())
	{
		ParticlePanel->Stop();
	}
}
void FUIParticleEmitterToolKit::StopEmit()
{
	if (AssetBeingEdited && ParticlePanel.IsValid())
	{
		ParticlePanel->StopEmit();
	}
}

EActiveTimerReturnType FUIParticleEmitterToolKit::UpdateWidgetFunc(double InCurrentTime, float InDeltaTime)
{
	return EActiveTimerReturnType::Continue;
}

void FUIParticleEmitterToolKit::CaptureThumbnail()
{
	const FGeometry& Geometry = ParticlePanel->GetCachedGeometry();
	FVector2D DrawSize = Geometry.GetAbsoluteSize();
	DrawSize = FVector2D(int32(DrawSize.X), int32(DrawSize.Y));
	if (!WidgetRenderer.IsValid())
		WidgetRenderer = MakeShareable(new FWidgetRenderer(true));

	UTextureRenderTarget2D* RenderTarget = WidgetRenderer->CreateTargetFor(DrawSize, TF_Bilinear, false);
	TSharedRef<SVirtualWindow> Window = SNew(SVirtualWindow).Size(DrawSize);
	TSharedRef<FHittestGrid> HitTestGrid = MakeShareable(new FHittestGrid());
	//Window->ReshapeWindow(WindowPosition, DrawSize);
	Window->Resize(DrawSize);
	Window->SetContent(ParticlePanel.ToSharedRef());
	const FGeometry& WindowGeometry = Geometry.MakeRoot(DrawSize, FSlateLayoutTransform(FVector2D::ZeroVector));

	FVector2D WindowPosition = Geometry.LocalToAbsolute(FVector2D::ZeroVector);
	const FPaintArgs PaintArgs = FPaintArgs(ParticlePanel.Get(), HitTestGrid.Get(), WindowPosition, 0, 0);
	WidgetRenderer->DrawWindow(PaintArgs, RenderTarget, Window, WindowGeometry, FSlateRect(0, 0, 0, 0), 0);

	int32 Width = DrawSize.X;
	int32 Height = DrawSize.Y;

	FVector2D ThumbnailSize = Width < Height ? FVector2D(Width, Width) : FVector2D(Height, Height);
	FVector2D LeftTop = Width < Height ? FVector2D(0, int32((Height - Width) / 2)) : FVector2D(int32((Width - Height) / 2), 0);

	TArray<uint8> RawData;
	bool bSuccess = GetRawData(RenderTarget, RawData);

	//setup actual thumbnail
	FObjectThumbnail TempThumbnail;
	TempThumbnail.SetImageSize(ThumbnailSize.X, ThumbnailSize.Y);
	TArray<uint8>& ThumbnailByteArray = TempThumbnail.AccessImageData();

	// Copy image into destination thumb
	int32 MemorySize = ThumbnailSize.X * ThumbnailSize.Y * sizeof(FColor);
	ThumbnailByteArray.AddUninitialized(MemorySize);
	int32 ThumbnailLineSize = ThumbnailSize.X * sizeof(FColor);
	int32 SourceLineSize = Width * sizeof(FColor);
	int32 SourceStart = LeftTop.Y * Width * sizeof(FColor) + LeftTop.X * sizeof(FColor);
	for (int32 i = 0; i < ThumbnailSize.Y; i++)
	{
		FMemory::Memcpy(&(ThumbnailByteArray[i*ThumbnailLineSize]), &(RawData[SourceStart + i * SourceLineSize]), ThumbnailLineSize);
	}

	//
	FAssetData CurrentAsset = FAssetData(AssetBeingEdited);
	const FString ObjectFullName = CurrentAsset.GetFullName();
	const FString PackageName = CurrentAsset.PackageName.ToString();

	UPackage* AssetPackage = FindObject<UPackage>(NULL, *PackageName);
	if (AssetPackage)
	{
		FObjectThumbnail* NewThumbnail = ThumbnailTools::CacheThumbnail(ObjectFullName, &TempThumbnail, AssetPackage);
		if (NewThumbnail)
		{
			//we need to indicate that the package needs to be resaved
			AssetBeingEdited->MarkPackageDirty();

			// Let the content browser know that we've changed the thumbnail
			NewThumbnail->MarkAsDirty();

			// Signal that the asset was changed if it is loaded so thumbnail pools will update
			AssetBeingEdited->PostEditChange();

			//Set that thumbnail as a valid custom thumbnail so it'll be saved out
			NewThumbnail->SetCreatedAfterCustomThumbsEnabled();
		}
	}
}
void FUIParticleEmitterToolKit::ClearThumbnail()
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	FAssetData CurrentAsset = FAssetData(AssetBeingEdited);

	// check whether this is a type that uses one of the shared static thumbnails
	if (AssetToolsModule.Get().AssetUsesGenericThumbnail(CurrentAsset))
	{
		//assign the thumbnail and dirty
		const FString ObjectFullName = CurrentAsset.GetFullName();
		const FString PackageName = CurrentAsset.PackageName.ToString();

		UPackage* AssetPackage = FindObject<UPackage>(NULL, *PackageName);
		if (ensure(AssetPackage))
		{
			ThumbnailTools::CacheEmptyThumbnail(ObjectFullName, AssetPackage);

			//we need to indicate that the package needs to be resaved
			AssetPackage->MarkPackageDirty();

			// Signal that the asset was changed if it is loaded so thumbnail pools will update
			if (CurrentAsset.IsAssetLoaded())
			{
				CurrentAsset.GetAsset()->PostEditChange();
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE
