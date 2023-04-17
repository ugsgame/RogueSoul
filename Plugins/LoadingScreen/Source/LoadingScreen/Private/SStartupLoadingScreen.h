// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "LoadingScreenSettings.h"

class SCircularThrobber;
class STextBlock;
class FDeferredCleanupSlateBrush;

class SStartupLoadingScreen : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SStartupLoadingScreen) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FLoadingScreenDescription& ScreenDescription);
	void ShowLoadingComplete(FText LoadCompletePrompt);
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
private:
	TSharedPtr<SCircularThrobber> LoadingIcon;
	TSharedPtr<STextBlock> LoadingText;

	float GetDPIScale() const;
	TOptional<float> GetProgressBarValue() const;
	
private:
	TSharedPtr<FDeferredCleanupSlateBrush> LoadingScreenBrush;
	TSharedPtr<FDeferredCleanupSlateBrush> BackgroundBrush;
	TSharedPtr<FDeferredCleanupSlateBrush> FillBrush;
	float FillPercent;
	float FillTime;
};
