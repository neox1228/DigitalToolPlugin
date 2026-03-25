#include "Features/AxisX/AxisXWidget.h"

#include "Features/AxisX/AxisXProxy.h"
#include "Features/AxisX/AxisXState.h"
#include "Rendering/DrawElements.h"

void SAxisXWidget::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._Proxy;
	if (!Proxy.IsValid())
	{
		Proxy = MakeShared<FAxisXProxy>();
	}

	StateChangedHandle = Proxy->OnStateChanged().AddSP(this, &SAxisXWidget::HandleStateChanged);
}

SAxisXWidget::~SAxisXWidget()
{
	if (Proxy.IsValid())
	{
		Proxy->OnStateChanged().Remove(StateChangedHandle);
	}
}

int32 SAxisXWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FAxisXState& State = Proxy->GetState();
	const FVector2D Size = FVector2D(AllottedGeometry.GetLocalSize());
	const FVector2D Padding = State.Padding;
	const FVector2D DrawMin = Padding;
	const FVector2D DrawMax = FVector2D(Size.X - Padding.X, Size.Y - Padding.Y);

	TArray<FVector2D> AxisPoints;
	AxisPoints.Add(FVector2D(DrawMin.X, DrawMax.Y));
	AxisPoints.Add(FVector2D(DrawMax.X, DrawMax.Y));

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		AxisPoints,
		ESlateDrawEffect::None,
		State.AxisColor,
		true,
		State.AxisThickness);

	return LayerId;
}

FVector2D SAxisXWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1.0f, 1.0f);
}

void SAxisXWidget::HandleStateChanged()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}
