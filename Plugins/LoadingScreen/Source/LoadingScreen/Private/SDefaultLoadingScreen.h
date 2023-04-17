// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "LoadingScreenSettings.h"

class SCircularThrobber;
class FDeferredCleanupSlateBrush;

class SDefaultLoadingScreen : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SDefaultLoadingScreen) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const FLoadingScreenDescription& ScreenDescription);

	void UpdateLoadingText(FText NewLoadingText);

private:
	TSharedPtr<STextBlock> LoadingText;

	float GetDPIScale() const;
	TOptional<float> GetProgressBarValue() const;
	
private:
	TSharedPtr<FDeferredCleanupSlateBrush> LoadingScreenBrush;
	TSharedPtr<FDeferredCleanupSlateBrush> BackgroundBrush;
	TSharedPtr<FDeferredCleanupSlateBrush> FillBrush;
};
