// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#include "UIParticleFloatCurveCustomization.h"
#include "Curves/CurveFloat.h"
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

#define LOCTEXT_NAMESPACE "FUIParticleFloatCurveCustomization"

const FVector2D FUIParticleFloatCurveCustomization::DEFAULT_WINDOW_SIZE = FVector2D(800, 500);

TSharedRef<IPropertyTypeCustomization> FUIParticleFloatCurveCustomization::MakeInstance() 
{
	return MakeShareable( new FUIParticleFloatCurveCustomization );
}

FUIParticleFloatCurveCustomization::~FUIParticleFloatCurveCustomization()
{
	DestroyPopOutWindow();
}

FUIParticleFloatCurveCustomization::FUIParticleFloatCurveCustomization()
	: RuntimeCurve(NULL)
	, Owner(NULL)
	, ViewMinInput(0.0f)
	, ViewMaxInput(5.0f)
{
}

void FUIParticleFloatCurveCustomization::CustomizeHeader( TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils )
{
	this->StructPropertyHandle = InStructPropertyHandle;

	TArray<UObject*> OuterObjects;
	StructPropertyHandle->GetOuterObjects(OuterObjects);

	TArray<void*> StructPtrs;
	StructPropertyHandle->AccessRawData( StructPtrs );

	if (StructPtrs.Num() == 1)
	{
		static const FName XAxisName(TEXT("XAxisName"));
		static const FName YAxisName(TEXT("YAxisName"));

		TOptional<FString> XAxisString;
		if ( InStructPropertyHandle->HasMetaData(XAxisName) )
		{
			XAxisString = InStructPropertyHandle->GetMetaData(XAxisName);
		}

		TOptional<FString> YAxisString;
		if ( InStructPropertyHandle->HasMetaData(YAxisName) )
		{
			YAxisString = InStructPropertyHandle->GetMetaData(YAxisName);
		}

		RuntimeCurve = reinterpret_cast<FUIParticleFloatCurve*>(StructPtrs[0]);

		if (OuterObjects.Num() == 1)
		{
			Owner = OuterObjects[0];
		}

		HeaderRow
			.NameContent()
			[
				InStructPropertyHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MinDesiredWidth(200)
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill)
				.OnMouseDoubleClick(this, &FUIParticleFloatCurveCustomization::OnCurvePreviewDoubleClick)
				[
					SAssignNew(CurveWidget, SCurveEditor)
					.ViewMinInput(this, &FUIParticleFloatCurveCustomization::GetViewMinInput)
					.ViewMaxInput(this, &FUIParticleFloatCurveCustomization::GetViewMaxInput)
					.TimelineLength(this, &FUIParticleFloatCurveCustomization::GetTimelineLength)
					.OnSetInputViewRange(this, &FUIParticleFloatCurveCustomization::SetInputViewRange)
					.XAxisName(XAxisString)
					.YAxisName(YAxisString)
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
				InStructPropertyHandle->CreatePropertyNameWidget()
			]
			.ValueContent()
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill)
				[
					SNew(STextBlock)
					.Text(StructPtrs.Num() == 0 ? LOCTEXT("NoCurves", "No Curves - unable to modify") : LOCTEXT("MultipleCurves", "Multiple Curves - unable to modify"))
				]
			];
	}
}

void FUIParticleFloatCurveCustomization::CustomizeChildren( TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils )
{
	uint32 NumChildren = 0;
	StructPropertyHandle->GetNumChildren(NumChildren);

	for( uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex )
	{
		TSharedPtr<IPropertyHandle> Child = StructPropertyHandle->GetChildHandle( ChildIndex );

		if( Child->GetProperty()->GetName() == TEXT("ExternalCurve") )
		{
			ExternalCurveHandle = Child;

			FSimpleDelegate OnCurveChangedDelegate = FSimpleDelegate::CreateSP( this, &FUIParticleFloatCurveCustomization::OnExternalCurveChanged, InStructPropertyHandle );
			Child->SetOnPropertyValueChanged(OnCurveChangedDelegate);

			StructBuilder.AddCustomRow(LOCTEXT("ExternalCurveLabel", "ExternalCurve"))
				.NameContent()
				[
					Child->CreatePropertyNameWidget()
				]
				.ValueContent()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						[
							Child->CreatePropertyValueWidget()
						]
						+SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(1,0)
						[
							SNew(SButton)
							.ButtonStyle( FEditorStyle::Get(), "NoBorder" )
							.ContentPadding(1.f)
							.ToolTipText(LOCTEXT("ConvertInternalCurveTooltip", "Convert to Internal Curve"))
							[
								SNew(SImage)
								.Image( FEditorStyle::GetBrush(TEXT("PropertyWindow.Button_Clear")) )
							]
						]
					]
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.Text( LOCTEXT( "CreateAssetButton", "Create External Curve" ) )
							.ToolTipText(LOCTEXT( "CreateAssetTooltip", "Create a new CurveFloat asset from this curve") )
							.Visibility(Owner != nullptr ? EVisibility::Visible : EVisibility::Collapsed)
						]
					]
				];
		}
		else
		{
			StructBuilder.AddProperty(Child.ToSharedRef());
		}
	}
}

TArray<FRichCurveEditInfoConst> FUIParticleFloatCurveCustomization::GetCurves() const
{
	TArray<FRichCurveEditInfoConst> Curves;
	Curves.Add(FRichCurveEditInfoConst(&RuntimeCurve->CurveData));
	return Curves;
}

TArray<FRichCurveEditInfo> FUIParticleFloatCurveCustomization::GetCurves()
{
	TArray<FRichCurveEditInfo> Curves;
	Curves.Add(FRichCurveEditInfo(&RuntimeCurve->CurveData));
	return Curves;
}

void FUIParticleFloatCurveCustomization::ModifyOwner()
{
	if (Owner)
	{
		Owner->Modify(true);
	}
}

TArray<const UObject*> FUIParticleFloatCurveCustomization::GetOwners() const
{
	TArray<const UObject*> Owners;
	if (Owner)
	{
		Owners.Add(Owner);
	}

	return Owners;
}

void FUIParticleFloatCurveCustomization::MakeTransactional()
{
	if (Owner)
	{
		Owner->SetFlags(Owner->GetFlags() | RF_Transactional);
	}
}

void FUIParticleFloatCurveCustomization::OnCurveChanged(const TArray<FRichCurveEditInfo>& ChangedCurveEditInfos)
{
	StructPropertyHandle->NotifyPostChange();
}

bool FUIParticleFloatCurveCustomization::IsValidCurve( FRichCurveEditInfo CurveInfo )
{
	return CurveInfo.CurveToEdit == &RuntimeCurve->CurveData;
}

float FUIParticleFloatCurveCustomization::GetTimelineLength() const
{
	return 0.f;
}

void FUIParticleFloatCurveCustomization::SetInputViewRange(float InViewMinInput, float InViewMaxInput)
{
	ViewMaxInput = InViewMaxInput;
	ViewMinInput = InViewMinInput;
}

void FUIParticleFloatCurveCustomization::OnExternalCurveChanged(TSharedRef<IPropertyHandle> CurvePropertyHandle)
{
	if (RuntimeCurve)
	{
		CurveWidget->SetCurveOwner(this);
		CurvePropertyHandle->NotifyPostChange();
	}
}

FReply FUIParticleFloatCurveCustomization::OnCurvePreviewDoubleClick( const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent )
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		DestroyPopOutWindow();

		// Determine the position of the window so that it will spawn near the mouse, but not go off the screen.
		const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
		FSlateRect Anchor(CursorPos.X, CursorPos.Y, CursorPos.X, CursorPos.Y);

		FVector2D AdjustedSummonLocation = FSlateApplication::Get().CalculatePopupWindowPosition( Anchor, FUIParticleFloatCurveCustomization::DEFAULT_WINDOW_SIZE, true, FVector2D::ZeroVector, Orient_Horizontal );

		TSharedPtr<SWindow> Window = SNew(SWindow)
			.Title( FText::Format( LOCTEXT("WindowHeader", "{0} - Internal Curve Editor"), StructPropertyHandle->GetPropertyDisplayName()) )
			.ClientSize( FUIParticleFloatCurveCustomization::DEFAULT_WINDOW_SIZE )
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

void FUIParticleFloatCurveCustomization::CopyCurveData( const FRichCurve* SrcCurve, FRichCurve* DestCurve )
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

void FUIParticleFloatCurveCustomization::DestroyPopOutWindow()
{
	if (CurveEditorWindow.IsValid())
	{
		CurveEditorWindow.Pin()->RequestDestroyWindow();
		CurveEditorWindow.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
