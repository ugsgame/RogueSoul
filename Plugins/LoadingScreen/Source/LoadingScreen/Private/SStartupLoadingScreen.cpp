// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SStartupLoadingScreen.h"

#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Engine/Texture2D.h"
#include "Engine/UserInterfaceSettings.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "Widgets/Notifications/SProgressBar.h"

float PointSizeToSlateUnits(float PointSize);

#define LOCTEXT_NAMESPACE "LoadingScreen"

/////////////////////////////////////////////////////
// SStartupLoadingScreen

void SStartupLoadingScreen::Construct(const FArguments& InArgs, const FLoadingScreenDescription& InScreenDescription)
{
	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();

	const FSlateFontInfo& TipFont = Settings->TipFont;
	const FSlateFontInfo& LoadingFont = Settings->LoadingFont;
	const float OffsetX = Settings->OffsetX;
	const float OffsetY = Settings->OffsetY;
	const float TextOffsetY = Settings->TextOffsetY;

	TSharedRef<SOverlay> Root = SNew(SOverlay);

	FillTime = InScreenDescription.FillTime;

	// If there's an image defined
	if ( InScreenDescription.Images.Num() > 0 )
	{
		const int32 ImageIndex = FMath::RandRange(0, InScreenDescription.Images.Num() - 1);
		const FStringAssetReference& ImageAsset = InScreenDescription.Images[ImageIndex];
		UObject* ImageObject = ImageAsset.TryLoad();
		if ( UTexture2D* LoadingImage = Cast<UTexture2D>(ImageObject) )
		{
			LoadingScreenBrush = FDeferredCleanupSlateBrush::CreateBrush(LoadingImage);

			Root->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBorder)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.BorderBackgroundColor(InScreenDescription.BackgroundColor)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				[
					SNew(SScaleBox)
					.Stretch(InScreenDescription.ImageStretch)
					[
						SNew(SImage)
						.Image(LoadingScreenBrush.IsValid() ? LoadingScreenBrush->GetSlateBrush() : nullptr)
					]
				]
			];
		}
	}

	

	const FStringAssetReference& BackgroundImageAsset = InScreenDescription.BackgroundImage;
	UObject * BackgroundImageObject = BackgroundImageAsset.TryLoad();
	const FStringAssetReference& FillImageAsset = InScreenDescription.FillImage;
	UObject* FillImageObject = FillImageAsset.TryLoad();
	if (UTexture2D* BackgroundImage = Cast<UTexture2D>(BackgroundImageObject))
	{
		if (UTexture2D* FillImage = Cast<UTexture2D>(FillImageObject))
		{
			BackgroundBrush = FDeferredCleanupSlateBrush::CreateBrush(BackgroundImage);
			FillBrush = FDeferredCleanupSlateBrush::CreateBrush(FillImage);

			Root->AddSlot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Bottom)
				[
					SNew(SSafeZone)
					.Padding(FMargin(0, 0, 0, OffsetY))
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Bottom)
					.IsTitleSafe(true)
					[
						SNew(SDPIScaler)
						.DPIScale(this, &SStartupLoadingScreen::GetDPIScale)
						[
							SNew(SOverlay)

							+ SOverlay::Slot()
							.Padding(FMargin(OffsetX, 0, OffsetX, 0))
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Center)
							[

								SNew(SProgressBar)
								.Percent(this, &SStartupLoadingScreen::GetProgressBarValue)
								.BackgroundImage(BackgroundBrush.IsValid() ? BackgroundBrush->GetSlateBrush() : nullptr)
								.FillImage(FillBrush.IsValid() ? FillBrush->GetSlateBrush() : nullptr)

							]

							+ SOverlay::Slot()
							.Padding(FMargin(OffsetX, TextOffsetY, 0, 0))
							.HAlign(HAlign_Left)
							.VAlign(VAlign_Center)
							[
								SAssignNew(LoadingText, STextBlock)
								.Text(InScreenDescription.LoadingText)
								.Font(LoadingFont)
							]
						]
					]
				];
		}
		
	}

	ChildSlot
	[
		Root
	];
}

TOptional<float> SStartupLoadingScreen::GetProgressBarValue() const
{
	return TOptional<float>(FillPercent);
}

void SStartupLoadingScreen::ShowLoadingComplete(FText LoadCompletePrompt)
{
	if (LoadingIcon.IsValid())
		LoadingIcon->SetVisibility(EVisibility::Hidden);

	if (LoadingText.IsValid())
		LoadingText->SetText(LoadCompletePrompt);
}

void SStartupLoadingScreen::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	FillPercent += InDeltaTime / FillTime;
	FillPercent = FMath::Clamp<float>(FillPercent, 0.f, 1.f);
}

float SStartupLoadingScreen::GetDPIScale() const
{
	const FVector2D& DrawSize = GetCachedGeometry().ToPaintGeometry().GetLocalSize();
	const FIntPoint Size((int32)DrawSize.X, (int32)DrawSize.Y);
	return GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(Size);
}

#undef LOCTEXT_NAMESPACE
