// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleEditorStyle.h"
#include "UIParticleEditorPrivatePCH.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Framework/Application/SlateApplication.h"

TSharedPtr< FSlateStyleSet > FUIParticleEditorStyle::StyleInstance = NULL;

void FUIParticleEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FUIParticleEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FUIParticleEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("UIParticleEditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FUIParticleEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("UIParticleEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UIParticle")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ParticleAsset.AssetActions.Play.Large", new IMAGE_BRUSH("PlayButtonIcon", Icon40x40));
	Style->Set("ParticleAsset.AssetActions.Stop.Large", new IMAGE_BRUSH("StopButtonIcon", Icon40x40));
	Style->Set("ParticleAsset.AssetActions.CaptureThumbnail.Large", new IMAGE_BRUSH("CaptureThumbnailButtonIcon", Icon40x40));
	Style->Set("ParticleAsset.AssetActions.ClearThumbnail.Large", new IMAGE_BRUSH("ClearThumbnailButtonIcon", Icon40x40));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FUIParticleEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FUIParticleEditorStyle::Get()
{
	return *StyleInstance;
}
