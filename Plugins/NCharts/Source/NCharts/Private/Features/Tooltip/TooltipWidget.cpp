// Copyright NCharts Plugin. All Rights Reserved.

#include "Features/Tooltip/TooltipWidget.h"

#include "Core/NChartCartesianScale.h"
#include "Core/NChartLayoutUtils.h"
#include "Core/NChartTooltipTypes.h"
#include "Core/NChartTypes.h"
#include "Features/Tooltip/TooltipProxy.h"
#include "Features/Tooltip/TooltipState.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

namespace
{
	INChartTooltipDataProvider* ResolveProvider(const TSharedPtr<INChartProxy>& Proxy)
	{
		return Proxy.IsValid() ? Proxy->GetTooltipDataProvider() : nullptr;
	}

	FText BuildCombinedTooltipText(const TArray<FTooltipParam>& Params)
	{
		if (Params.Num() == 0)
		{
			return FText::GetEmpty();
		}

		if (Params.Num() == 1)
		{
			return Params[0].DisplayText;
		}

		FString Combined;
		for (int32 Index = 0; Index < Params.Num(); ++Index)
		{
			if (Index > 0)
			{
				Combined += TEXT("\n\n");
			}
			Combined += Params[Index].DisplayText.ToString();
		}
		return FText::FromString(Combined);
	}

	FVector2D GetTooltipAnchor(const FTooltipState& TooltipState)
	{
		if (TooltipState.ActiveParams.Num() == 0)
		{
			return FVector2D::ZeroVector;
		}

		if (TooltipState.Trigger == EChartTooltipTrigger::Axis)
		{
			const FVector2D& FirstPoint = TooltipState.ActiveParams[0].ScreenPoint;
			return FVector2D(TooltipState.AxisScreenX, FirstPoint.Y);
		}

		return TooltipState.ActiveParams[0].ScreenPoint;
	}
}

void STooltipWidget::Construct(const FArguments& InArgs)
{
	Proxy = InArgs._Proxy;
	if (!Proxy.IsValid())
	{
		Proxy = MakeShared<FTooltipProxy>();
	}

	StateChangedHandle = Proxy->OnStateChanged().AddSP(this, &STooltipWidget::HandleStateChanged);
}

STooltipWidget::~STooltipWidget()
{
	if (Proxy.IsValid())
	{
		Proxy->OnStateChanged().Remove(StateChangedHandle);
	}
}

int32 STooltipWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const FTooltipState& TooltipState = Proxy->GetState();
	if (!TooltipState.bEnableTooltip || !TooltipState.bHasHover || TooltipState.ActiveParams.Num() == 0)
	{
		return LayerId;
	}

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FVector2D AnchorPoint = GetTooltipAnchor(TooltipState);
	const float LineHeight = 18.0f;
	const FVector2D TooltipSize(160.0f, 36.0f + FMath::Max(0, TooltipState.ActiveParams.Num() - 1) * LineHeight);
	const FVector2D TooltipPos = AnchorPoint + TooltipState.TooltipOffset;

	if (TooltipState.bShowHoverLine)
	{
		const float LineX = TooltipState.Trigger == EChartTooltipTrigger::Axis
			? TooltipState.AxisScreenX
			: TooltipState.ActiveParams[0].ScreenPoint.X;

		TArray<FVector2D> HoverLinePoints;
		HoverLinePoints.Add(FVector2D(LineX, TooltipState.DrawMin.Y));
		HoverLinePoints.Add(FVector2D(LineX, TooltipState.DrawMax.Y));
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(),
			HoverLinePoints,
			ESlateDrawEffect::None,
			TooltipState.HoverLineColor,
			true,
			1.0f);
	}

	if (TooltipState.bShowHoverPoint)
	{
		for (const FTooltipParam& Param : TooltipState.ActiveParams)
		{
			const FVector2D MarkerSize(TooltipState.HoverPointRadius * 2.0f, TooltipState.HoverPointRadius * 2.0f);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(
					FVector2D(Param.ScreenPoint.X - TooltipState.HoverPointRadius, Param.ScreenPoint.Y - TooltipState.HoverPointRadius),
					MarkerSize),
				WhiteBrush,
				ESlateDrawEffect::None,
				TooltipState.HoverPointColor);
		}
	}

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId + 3,
		AllottedGeometry.ToPaintGeometry(TooltipPos, TooltipSize),
		WhiteBrush,
		ESlateDrawEffect::None,
		TooltipState.TooltipBackgroundColor);

	const FText TooltipText = BuildCombinedTooltipText(TooltipState.ActiveParams);
	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId + 4,
		AllottedGeometry.ToPaintGeometry(FVector2D(TooltipPos.X + 8.0f, TooltipPos.Y + 6.0f), TooltipSize),
		TooltipText,
		FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10),
		ESlateDrawEffect::None,
		TooltipState.TooltipTextColor);

	return LayerId + 4;
}

FReply STooltipWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!Proxy.IsValid())
	{
		return FReply::Unhandled();
	}

	const FTooltipState& TooltipState = Proxy->GetState();
	if (!TooltipState.bEnableTooltip)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	if (!UpdateHoverState(MyGeometry, LocalMousePosition))
	{
		return FReply::Unhandled();
	}

	Invalidate(EInvalidateWidgetReason::Paint);
	return FReply::Handled();
}

void STooltipWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	ClearHoverState();
	Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D STooltipWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(1.0f, 1.0f);
}

void STooltipWidget::HandleStateChanged()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}

bool STooltipWidget::UpdateHoverState(const FGeometry& MyGeometry, const FVector2D& LocalMousePosition)
{
	const FTooltipState& TooltipState = Proxy->GetState();
	const FVector2D GeometrySize = FVector2D(MyGeometry.GetLocalSize());
	const float ActivationDistSq = FMath::Square(TooltipState.ActivationDistance);

	if (TooltipState.Trigger == EChartTooltipTrigger::Axis)
	{
		return UpdateAxisHoverState(GeometrySize, LocalMousePosition);
	}

	return UpdateItemHoverState(GeometrySize, LocalMousePosition, ActivationDistSq);
}

bool STooltipWidget::UpdateItemHoverState(
	const FVector2D& GeometrySize,
	const FVector2D& LocalMousePosition,
	float ActivationDistSq)
{
	const TSharedPtr<FNChartCartesianScale> SharedScale = Proxy->GetCartesianScale();
	if (SharedScale.IsValid())
	{
		SharedScale->UpdatePixelRect(SharedScale->Padding, GeometrySize);
	}

	FTooltipParam BestParam;
	float BestDistanceSq = TNumericLimits<float>::Max();
	FVector2D DrawMin = FVector2D::ZeroVector;
	FVector2D DrawMax = FVector2D::ZeroVector;
	bool bFound = false;

	for (const TWeakPtr<INChartProxy>& WeakProvider : Proxy->GetDataProviders())
	{
		const TSharedPtr<INChartProxy> ProviderProxy = WeakProvider.Pin();
		INChartTooltipDataProvider* Provider = ResolveProvider(ProviderProxy);
		if (!Provider)
		{
			continue;
		}

		TArray<FVector2D> ScreenPoints;
		if (SharedScale.IsValid())
		{
			SharedScale->BuildScreenPoints(Provider->GetDataPoints(), ScreenPoints);
			DrawMin = SharedScale->DrawMin;
			DrawMax = SharedScale->DrawMax;
		}
		else
		{
			FNChartScreenLayout Layout;
			if (!FNChartLayoutUtils::BuildScreenLayout(
				Provider->GetDataPoints(),
				Provider->GetChartPadding(),
				GeometrySize,
				Provider->GetMinPointCountForLayout(),
				Layout))
			{
				continue;
			}
			ScreenPoints = Layout.ScreenPoints;
			DrawMin = Layout.DrawMin;
			DrawMax = Layout.DrawMax;
		}

		float DistanceSq = 0.0f;
		const int32 PointIndex = FNChartLayoutUtils::FindNearestByScreenPosition(ScreenPoints, LocalMousePosition, DistanceSq);
		if (PointIndex == INDEX_NONE || DistanceSq >= BestDistanceSq)
		{
			continue;
		}

		BestDistanceSq = DistanceSq;
		BestParam.SeriesName = Provider->GetSeriesName();
		BestParam.FeatureType = Provider->GetProviderFeatureType();
		BestParam.PointIndex = PointIndex;
		BestParam.DataPoint = Provider->GetDataPoints()[PointIndex];
		BestParam.ScreenPoint = ScreenPoints[PointIndex];
		BestParam.DisplayText = Provider->FormatTooltipText(PointIndex);
		bFound = true;
	}

	if (!bFound || BestDistanceSq > ActivationDistSq)
	{
		ClearHoverState();
		return false;
	}

	TArray<FTooltipParam> Params;
	Params.Add(BestParam);
	Proxy->SetActiveTooltip(Params, BestParam.ScreenPoint.X, DrawMin, DrawMax);
	return true;
}

bool STooltipWidget::UpdateAxisHoverState(const FVector2D& GeometrySize, const FVector2D& LocalMousePosition)
{
	const TSharedPtr<FNChartCartesianScale> SharedScale = Proxy->GetCartesianScale();
	if (!SharedScale.IsValid())
	{
		ClearHoverState();
		return false;
	}

	SharedScale->UpdatePixelRect(SharedScale->Padding, GeometrySize);

	float DataX = 0.0f;
	if (!SharedScale->ScreenXToDataX(LocalMousePosition.X, DataX))
	{
		ClearHoverState();
		return false;
	}

	TArray<FTooltipParam> Params;
	for (const TWeakPtr<INChartProxy>& WeakProvider : Proxy->GetDataProviders())
	{
		const TSharedPtr<INChartProxy> ProviderProxy = WeakProvider.Pin();
		INChartTooltipDataProvider* Provider = ResolveProvider(ProviderProxy);
		if (!Provider)
		{
			continue;
		}

		const int32 PointIndex = FNChartLayoutUtils::FindNearestByDataX(Provider->GetDataPoints(), DataX);
		if (PointIndex == INDEX_NONE)
		{
			continue;
		}

		TArray<FVector2D> ScreenPoints;
		SharedScale->BuildScreenPoints(Provider->GetDataPoints(), ScreenPoints);
		if (!ScreenPoints.IsValidIndex(PointIndex))
		{
			continue;
		}

		FTooltipParam Param;
		Param.SeriesName = Provider->GetSeriesName();
		Param.FeatureType = Provider->GetProviderFeatureType();
		Param.PointIndex = PointIndex;
		Param.DataPoint = Provider->GetDataPoints()[PointIndex];
		Param.ScreenPoint = ScreenPoints[PointIndex];
		Param.DisplayText = Provider->FormatTooltipText(PointIndex);
		Params.Add(Param);
	}

	if (Params.Num() == 0)
	{
		ClearHoverState();
		return false;
	}

	const float AxisScreenX = FMath::Clamp(LocalMousePosition.X, SharedScale->DrawMin.X, SharedScale->DrawMax.X);
	Proxy->SetActiveTooltip(Params, AxisScreenX, SharedScale->DrawMin, SharedScale->DrawMax);
	return true;
}

void STooltipWidget::ClearHoverState()
{
	if (Proxy.IsValid())
	{
		Proxy->ClearHoverState();
	}
}
