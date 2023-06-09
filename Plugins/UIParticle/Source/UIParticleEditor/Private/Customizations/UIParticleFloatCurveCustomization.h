// Copyright (C) 2018-2019, RedStarStudio, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"
#include "Curves/CurveOwnerInterface.h"
#include "IDetailChildrenBuilder.h"

class FDetailWidgetRow;
class SCurveEditor;
struct FUIParticleFloatCurve;

/**
 * Customizes a RuntimeFloatCurve struct to display a Curve Editor
 */
class FUIParticleFloatCurveCustomization : public IPropertyTypeCustomization, public FCurveOwnerInterface
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/**
	 * Destructor
	 */
	virtual ~FUIParticleFloatCurveCustomization();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader( TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;
	virtual void CustomizeChildren( TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;

	/** FCurveOwnerInterface interface */
	virtual TArray<FRichCurveEditInfoConst> GetCurves() const override;
	virtual TArray<FRichCurveEditInfo> GetCurves() override;
	virtual void ModifyOwner() override;
	virtual TArray<const UObject*> GetOwners() const override;
	virtual void MakeTransactional() override;
	virtual void OnCurveChanged(const TArray<FRichCurveEditInfo>& ChangedCurveEditInfos) override;
	virtual bool IsValidCurve( FRichCurveEditInfo CurveInfo ) override;

private:
	/**
	 * Constructor
	 */
	FUIParticleFloatCurveCustomization();

	/**
	 * Get View Min Input for the Curve Editor
	 */
	float GetViewMinInput() const { return ViewMinInput; }

	/**
	 * Get View Min Input for the Curve Editor
	 */
	float GetViewMaxInput() const { return ViewMaxInput; }

	/**
	 * Get Timeline Length for the Curve Editor
	 */
	float GetTimelineLength() const;

	/**
	 * Set View Min and Max Inputs for the Curve Editor
	 */
	void SetInputViewRange(float InViewMinInput, float InViewMaxInput);

	/**
	 * Called when RuntimeFloatCurve's External Curve is changed
	 */
	void OnExternalCurveChanged(TSharedRef<IPropertyHandle> CurvePropertyHandle);

	/**
	 * Called when user double clicks on the curve preview to open a full size editor
	 */
	FReply OnCurvePreviewDoubleClick( const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent );

	/**
	 * Copies data from one Rich Curve to another
	 */
	static void CopyCurveData( const FRichCurve* SrcCurve, FRichCurve* DestCurve );

	/**
	 * Destroys the pop out window used for editing internal curves.
	 */
	void DestroyPopOutWindow();

private:
	/** Cached RuntimeFloatCurve struct handle */
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	/** Cached External Curve handle */
	TSharedPtr<IPropertyHandle> ExternalCurveHandle;

	/** Small preview Curve Editor */
	TSharedPtr<SCurveEditor> CurveWidget;

	/** Window for pop out Curve Editor */
	TWeakPtr<SWindow> CurveEditorWindow;

	/** Cached pointer to the actual RuntimeFloatCurve struct */
	FUIParticleFloatCurve* RuntimeCurve;

	/** Object that owns the RuntimeFloatCurve */
	UObject* Owner;

	/** View Min Input for the Curve Editor */
	float ViewMinInput;

	/** View Max Input for the Curve Editor */
	float ViewMaxInput;

	/** Size of the pop out Curve Editor window */
	static const FVector2D DEFAULT_WINDOW_SIZE;
};
