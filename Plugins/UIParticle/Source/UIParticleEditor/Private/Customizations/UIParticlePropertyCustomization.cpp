// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticlePropertyCustomization.h"
#include "Engine/MeshMerging.h"
#include "Misc/Attribute.h"
#include "UObject/UnrealType.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailCategoryBuilder.h"
#include "PropertyRestriction.h"
#include "IDetailGroup.h"

#define LOCTEXT_NAMESPACE "FUIParticlePropertyCustomization"

FUIParticlePropertyCustomization::~FUIParticlePropertyCustomization()
{
}


void FUIParticlePropertyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);
	FString Name = StructPropertyHandle->GetPropertyDisplayName().ToString();
	IDetailGroup& Group = StructBuilder.AddGroup(FName(*Name), FText::FromString(Name));

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedRef<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();
		//IDetailPropertyRow& Property = StructBuilder.AddProperty(ChildHandle);

		IDetailPropertyRow& Property = Group.AddPropertyRow(ChildHandle);

		if (ChildHandle->GetProperty()->GetFName() == "Type" )
		{
			EnumProperty = ChildHandle;
		}
		else
		{
			static const FName EditConditionName = "EnumCondition";
			int32 EnumCondition = ChildHandle->GetIntMetaData(EditConditionName);
			Property.Visibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &FUIParticlePropertyCustomization::ArePropertiesVisible, EnumCondition)));
		}

	}
}

TSharedRef<IPropertyTypeCustomization> FUIParticlePropertyCustomization::MakeInstance()
{
	return MakeShareable(new FUIParticlePropertyCustomization);
}

EVisibility FUIParticlePropertyCustomization::ArePropertiesVisible(const int32 VisibleType) const
{
	uint8 CurrentEnumValue = 0;
	EnumProperty->GetValue(CurrentEnumValue);
	return (CurrentEnumValue == VisibleType) ? EVisibility::Visible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
