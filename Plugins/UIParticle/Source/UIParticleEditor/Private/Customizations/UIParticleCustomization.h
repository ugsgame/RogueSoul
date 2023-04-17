// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IPropertyHandle;
class UUIParticle;

/**
 * Implements a details view customization for the UFileMediaSource class.
 */
class FUIParticleCustomization
	: public IDetailCustomization
{
public:

	//~ IDetailCustomization interface

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

public:

	/**
	 * Creates an instance of this class.
	 *
	 * @return The new instance.
	 */
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FUIParticleCustomization());
	}
	
protected:
	FReply OnPlayButtonClicked();
	FReply OnStopButtonClicked();
	FReply OnStopEmitButtonClicked();
protected:
	TArray<UUIParticle*> ModifierInstances;
};
