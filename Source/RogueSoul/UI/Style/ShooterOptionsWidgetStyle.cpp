// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "RogueSoul.h"
#include "ShooterOptionsWidgetStyle.h"

FShooterOptionsStyle::FShooterOptionsStyle()
{
}

FShooterOptionsStyle::~FShooterOptionsStyle()
{
}

const FName FShooterOptionsStyle::TypeName(TEXT("FShooterOptionsStyle"));

const FShooterOptionsStyle& FShooterOptionsStyle::GetDefault()
{
	static FShooterOptionsStyle Default;
	return Default;
}

void FShooterOptionsStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
}


UShooterOptionsWidgetStyle::UShooterOptionsWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
