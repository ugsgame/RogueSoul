// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleEmitterCustomization.h"
#include "UIParticleEditorPrivatePCH.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Widget/UIParticleEmitter.h"

#define LOCTEXT_NAMESPACE "FUIParticleEmitterCustomization"


/* IDetailCustomization interface
 *****************************************************************************/

void FUIParticleEmitterCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	//ensure(Objects.Num() == 1); // This is in here to ensure we are only showing the modifier details in the blueprint editor
	ModifierInstances.Empty();
	for (TWeakObjectPtr<UObject> Object : Objects)
	{
		if (Object->IsA<UUIParticleEmitter>())
		{
			UUIParticleEmitter * ModifierInstance = Cast<UUIParticleEmitter>(Object.Get());
			ModifierInstances.Add(ModifierInstance);
		}
	}

	// If we have found a valid modifier instance add a revision bump button to the details panel
	if (ModifierInstances.Num()>0)
	{
		IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory(TEXT("Particle"), LOCTEXT("ParticleSection", "Particle"), ECategoryPriority::Default);

		CategoryBuilder.AddCustomRow(LOCTEXT("PlayButtonLabel", "Play"))
			.WholeRowContent()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("PlayButtonLabel", "Play"))
					.ToolTipText(LOCTEXT("PlayButtonTooltip", "Play."))
					.OnClicked(this, &FUIParticleEmitterCustomization::OnPlayButtonClicked)
				]
				+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("StopButtonLabel", "Stop"))
					.ToolTipText(LOCTEXT("StopButtonTooltip", "Stop."))
					.OnClicked(this, &FUIParticleEmitterCustomization::OnStopButtonClicked)
					]
				+ SHorizontalBox::Slot()
					.HAlign(HAlign_Fill)
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("StopEmitButtonLabel", "StopEmit"))
					.ToolTipText(LOCTEXT("StopEmitButtonTooltip", "StopEmit."))
					.OnClicked(this, &FUIParticleEmitterCustomization::OnStopEmitButtonClicked)
					]
			];
	}
}

FReply FUIParticleEmitterCustomization::OnPlayButtonClicked()
{
	for (auto objPtr: ModifierInstances)
	{
		if (objPtr)
			objPtr->Play();
	}
	return FReply::Handled();
}

FReply FUIParticleEmitterCustomization::OnStopButtonClicked()
{
	for (auto objPtr : ModifierInstances)
	{
		if (objPtr)
			objPtr->Stop();
	}
	return FReply::Handled();
}

FReply FUIParticleEmitterCustomization::OnStopEmitButtonClicked()
{
	for (auto objPtr : ModifierInstances)
	{
		if (objPtr)
			objPtr->StopEmit();
	}
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE
