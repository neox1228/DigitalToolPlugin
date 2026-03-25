#include "Features/AxisY/AxisYWidget.h"

#include "Features/AxisY/AxisYProxy.h"
#include "Features/AxisY/AxisYState.h"
#include "Rendering/DrawElements.h"

void SAxisYWidget::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._Proxy;
	if (!Proxy.IsValid())
	{
		Proxy = MakeShared<FAxisYProxy>();
	}

	// AddSP(全称：AddSharedPointer)
	StateChangedHandle = Proxy->OnStateChanged().AddSP(this, &SAxisYWidget::HandleStateChanged);
}

SAxisYWidget::~SAxisYWidget()
{
	if (Proxy.IsValid())
	{
		Proxy->OnStateChanged().Remove(StateChangedHandle);
	}
}

int32 SAxisYWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FAxisYState& State = Proxy->GetState();
	const FVector2D Size = FVector2D(AllottedGeometry.GetLocalSize());
	const FVector2D Padding = State.Padding;
	const FVector2D DrawMin = Padding;
	const FVector2D DrawMax = FVector2D(Size.X - Padding.X, Size.Y - Padding.Y);

	TArray<FVector2D> AxisPoints;
	AxisPoints.Add(FVector2D(DrawMin.X, DrawMax.Y));
	AxisPoints.Add(FVector2D(DrawMin.X, DrawMin.Y));

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

FVector2D SAxisYWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1.0f, 1.0f);
}

void SAxisYWidget::HandleStateChanged()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}
