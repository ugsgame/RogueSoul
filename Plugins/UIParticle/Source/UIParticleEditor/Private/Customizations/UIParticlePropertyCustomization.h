// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Widgets/SWidget.h"
#include "Editor/PropertyEditor/Public/IPropertyTypeCustomization.h"


class IPropertyHandle;

class FUIParticlePropertyCustomization : public IPropertyTypeCustomization // public IDetailCustomization 
{
public:
	~FUIParticlePropertyCustomization();

	/** Begin IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {}
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	/** End IPropertyTypeCustomization interface */


	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
protected:
	EVisibility ArePropertiesVisible(const int32 VisibleType) const;
	TSharedPtr<IPropertyHandle> EnumProperty;
};
