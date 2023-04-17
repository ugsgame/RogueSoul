// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once
#include "Editor/UnrealEd/Classes/Factories/Factory.h"
#include "UIParticleAssetFactory.generated.h"
/** Factory for creating SlateBrushAssets */
UCLASS()
class  UUIParticleAssetFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	//virtual uint32 GetMenuCategories() const override;
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// Begin UFactory Interface
};


