#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Types/SlateEnums.h"
#include "Widgets/SLeafWidget.h"

class FActiveTimerHandle;
struct FPointerEvent;
class FPointBubblesProxy;

class NCHARTS_API SPointBubblesWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SPointBubblesWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FPointBubblesProxy>, Proxy)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SPointBubblesWidget() override;

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	void HandleStateChanged();
	EActiveTimerReturnType HandleBreathTick(double InCurrentTime, float InDeltaTime);
	bool UpdateHoverState(const FGeometry& MyGeometry, const FVector2D& LocalMousePosition);
	void ClearHoverState();
	bool BuildLayout(const FVector2D& GeometrySize, TArray<FVector2D>& OutScreenPoints) const;
	void DrawCircle(
		const FGeometry& AllottedGeometry,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FVector2D& Center,
		float Radius,
		const FLinearColor& Color) const;

	TSharedPtr<FPointBubblesProxy> Proxy;
	FDelegateHandle StateChangedHandle;
	TWeakPtr<FActiveTimerHandle> BreathTimerHandle;
};
