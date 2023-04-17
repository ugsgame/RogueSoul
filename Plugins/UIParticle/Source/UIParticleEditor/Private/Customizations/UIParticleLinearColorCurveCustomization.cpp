// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleLinearColorCurveCustomization.h"
#include "Curves/CurveLinearColor.h"
#include "Misc/PackageName.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor.h"
#include "Widgets/Layout/SBorder.h"
#include "DetailWidgetRow.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Dialogs/Dialogs.h"
#include "Toolkits/AssetEditorManager.h"
#include "Dialogs/DlgPickAssetPath.h"
#include "PackageTools.h"
#include "MiniCurveEditor.h"
#include "AssetRegistryModule.h"
#include "SCurveEditor.h"
#include "Engine/Selection.h"

#define LOCTEXT_NAMESPACE "CurveColorCustomization"

const FVector2D FUIParticleLinearColorCurveCustomization::DEFAULT_WINDOW_SIZE = FVector2D(800, 500);

TSharedRef<IPropertyTypeCustomization> FUIParticleLinearColorCurveCustomization::MakeInstance()
{
	return MakeShareable( new FUIParticleLinearColorCurveCustomization );
}

FUIParticleLinearColorCurveCustomization::~FUIParticleLinearColorCurveCustomization()
{
	DestroyPopOutWindow();
}

FUIParticleLinearColorCurveCustomization::FUIParticleLinearColorCurveCustomization()
	: RuntimeCurve(NULL)
	, Owner(NULL)
	, ViewMinInput(0.0f)
	, ViewMaxInput(5.0f)
{
}

void FUIParticleLinearColorCurveCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	this->StructPropertyHandle = InStructPropertyHandle;

	TArray<UObject*> OuterObjects;
	StructPropertyHandle->GetOuterObjects(OuterObjects);

	TArray<void*> StructPtrs;
	StructPropertyHandle->AccessRawData( StructPtrs );
	check(StructPtrs.Num()!=0);

	if (StructPtrs.Num() == 1)
	{
		RuntimeCurve = reinterpret_cast<FUIParticleLinearColorCurve*>(StructPtrs[0]);

		if (OuterObjects.Num() == 1)
		{
			Owner = OuterObjects[0];
		}

		HeaderRow
			.NameContent()
			[
				InStructPropertyHandle->CreatePropertyNameWidget( FText::GetEmpty(), FText::GetEmpty(), false )
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MinDesiredWidth(200)
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill)
				.OnMouseDoubleClick(this, &FUIParticleLinearColorCurveCustomization::OnCurvePreviewDoubleClick)
				[
					SAssignNew(CurveWidget, SCurveEditor)
					.ViewMinInput(this, &FUIParticleLinearColorCurveCustomization::GetViewMinInput)
					.ViewMaxInput(this, &FUIParticleLinearColorCurveCustomization::GetViewMaxInput)
					.TimelineLength(this, &FUIParticleLinearColorCurveCustomization::GetTimelineLength)
					.OnSetInputViewRange(this, &FUIParticleLinearColorCurveCustomization::SetInputViewRange)
					.HideUI(false)
					.DesiredSize(FVector2D(300, 150))
				]
			];

		check(CurveWidget.IsValid());
		CurveWidget->SetCurveOwner(this);
	}
	else
	{
		HeaderRow
			.NameContent()
			[
				InStructPropertyHandle->CreatePropertyNameWidget( FText::GetEmpty(), FText::GetEmpty(), false )
			]
			.ValueContent()
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("MultipleCurves", "Multiple Curves - unable to modify"))
				]
			];
	}
}

void FUIParticleLinearColorCurveCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	uint32 NumChildren = 0;
	StructPropertyHandle->GetNumChildren(NumChildren);

	for( uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex )
	{
		TSharedPtr<IPropertyHandle> Child = StructPropertyHandle->GetChildHandle( ChildIndex );
		StructBuilder.AddProperty(Child.ToSharedRef());
	}
}

static const FName RedCurveName(TEXT("R"));
static const FName GreenCurveName(TEXT("G"));
static const FName BlueCurveName(TEXT("B"));
static const FName AlphaCurveName(TEXT("A"));

TArray<FRichCurveEditInfoConst> FUIParticleLinearColorCurveCustomization::GetCurves() const
{
	TArray<FRichCurveEditInfoConst> Curves;
	Curves.Add(FRichCurveEditInfoConst(&RuntimeCurve->ColorCurves[0], RedCurveName));
	Curves.Add(FRichCurveEditInfoConst(&RuntimeCurve->ColorCurves[1], GreenCurveName));
	Curves.Add(FRichCurveEditInfoConst(&RuntimeCurve->ColorCurves[2], BlueCurveName));
	Curves.Add(FRichCurveEditInfoConst(&RuntimeCurve->ColorCurves[3], AlphaCurveName));
	return Curves;
}

TArray<FRichCurveEditInfo> FUIParticleLinearColorCurveCustomization::GetCurves()
{
	TArray<FRichCurveEditInfo> Curves;
	Curves.Add(FRichCurveEditInfo(&RuntimeCurve->ColorCurves[0], RedCurveName));
	Curves.Add(FRichCurveEditInfo(&RuntimeCurve->ColorCurves[1], GreenCurveName));
	Curves.Add(FRichCurveEditInfo(&RuntimeCurve->ColorCurves[2], BlueCurveName));
	Curves.Add(FRichCurveEditInfo(&RuntimeCurve->ColorCurves[3], AlphaCurveName));
	return Curves;
}

void FUIParticleLinearColorCurveCustomization::ModifyOwner()
{
	if (Owner)
	{
		Owner->Modify(true);
	}
}

TArray<const UObject*> FUIParticleLinearColorCurveCustomization::GetOwners() const
{
	TArray<const UObject*> Owners;
	if (Owner)
	{
		Owners.Add(Owner);
	}

	return Owners;
}

void FUIParticleLinearColorCurveCustomization::MakeTransactional()
{
	if (Owner)
	{
		Owner->SetFlags(Owner->GetFlags() | RF_Transactional);
	}
}

void FUIParticleLinearColorCurveCustomization::OnCurveChanged(const TArray<FRichCurveEditInfo>& ChangedCurveEditInfos)
{
	StructPropertyHandle->NotifyPostChange();
}

FLinearColor FUIParticleLinearColorCurveCustomization::GetLinearColorValue(float InTime) const
{
	if (RuntimeCurve)
	{
		return RuntimeCurve->GetValue(InTime, InTime, InTime);
	}
	return FLinearColor::Black; 
}

bool FUIParticleLinearColorCurveCustomization::HasAnyAlphaKeys() const
{
	if (RuntimeCurve)
	{
		return RuntimeCurve->ColorCurves[3].GetNumKeys() > 0;
	}
	return false;
}

bool FUIParticleLinearColorCurveCustomization::IsValidCurve( FRichCurveEditInfo CurveInfo )
{
	return 
		CurveInfo.CurveToEdit == &RuntimeCurve->ColorCurves[0] ||
		CurveInfo.CurveToEdit == &RuntimeCurve->ColorCurves[1] ||
		CurveInfo.CurveToEdit == &RuntimeCurve->ColorCurves[2] ||
		CurveInfo.CurveToEdit == &RuntimeCurve->ColorCurves[3];
}

float FUIParticleLinearColorCurveCustomization::GetTimelineLength() const
{
	return 0.f;
}

void FUIParticleLinearColorCurveCustomization::SetInputViewRange(float InViewMinInput, float InViewMaxInput)
{
	ViewMaxInput = InViewMaxInput;
	ViewMinInput = InViewMinInput;
}

void FUIParticleLinearColorCurveCustomization::OnExternalCurveChanged(TSharedRef<IPropertyHandle> CurvePropertyHandle)
{
	if (RuntimeCurve)
	{
		CurveWidget->SetCurveOwner(this);
		CurvePropertyHandle->NotifyPostChange();
	}
}

FReply FUIParticleLinearColorCurveCustomization::OnCurvePreviewDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		DestroyPopOutWindow();

		// Determine the position of the window so that it will spawn near the mouse, but not go off the screen.
		const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
		FSlateRect Anchor(CursorPos.X, CursorPos.Y, CursorPos.X, CursorPos.Y);

		FVector2D AdjustedSummonLocation = FSlateApplication::Get().CalculatePopupWindowPosition( Anchor, FUIParticleLinearColorCurveCustomization::DEFAULT_WINDOW_SIZE, true, FVector2D::ZeroVector, Orient_Horizontal );

		TSharedPtr<SWindow> Window = SNew(SWindow)
			.Title( FText::Format( LOCTEXT("WindowHeader", "{0} - Internal Color Curve Editor"), StructPropertyHandle->GetPropertyDisplayName()) )
			.ClientSize( FUIParticleLinearColorCurveCustomization::DEFAULT_WINDOW_SIZE )
			.ScreenPosition(AdjustedSummonLocation)
			.AutoCenter(EAutoCenter::None)
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.SizingRule( ESizingRule::FixedSize );

		// init the mini curve editor widget
		TSharedRef<SMiniCurveEditor> MiniCurveEditor =
			SNew(SMiniCurveEditor)
			.CurveOwner(this)
			.OwnerObject(Owner)
			.ParentWindow(Window);

		Window->SetContent( MiniCurveEditor );

		// Find the window of the parent widget
		FWidgetPath WidgetPath;
		FSlateApplication::Get().GeneratePathToWidgetChecked( CurveWidget.ToSharedRef(), WidgetPath );
		Window = FSlateApplication::Get().AddWindowAsNativeChild( Window.ToSharedRef(), WidgetPath.GetWindow() );

		//hold on to the window created for external use...
		CurveEditorWindow = Window;
	}
	return FReply::Handled();
}

void FUIParticleLinearColorCurveCustomization::CopyCurveData( const FRichCurve* SrcCurve, FRichCurve* DestCurve )
{
	if( SrcCurve && DestCurve )
	{
		for (auto It(SrcCurve->GetKeyIterator()); It; ++It)
		{
			const FRichCurveKey& Key = *It;
			FKeyHandle KeyHandle = DestCurve->AddKey(Key.Time, Key.Value);
			DestCurve->GetKey(KeyHandle) = Key;
		}
	}
}

void FUIParticleLinearColorCurveCustomization::DestroyPopOutWindow()
{
	if (CurveEditorWindow.IsValid())
	{
		CurveEditorWindow.Pin()->RequestDestroyWindow();
		CurveEditorWindow.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
