#include "Widget/Zoom/CartesianZoom.h"

#include "Frame/ChartFrame.h"
#include "Spec/NXChartSpec.h"

namespace
{
	/**
	 * 按轴边界钳制单侧或两侧。
	 * @return false 表示夹紧后与当前域没有区别（顶死在边界），这次缩放应被忽略
	 */
	bool ClampSide(const FChartAxisSpec& Axis, const FChartDomain& Current, FChartDomain& Domain)
	{
		bool bLower = false;
		bool bUpper = false;
		double Lower = 0.0;
		double Upper = 0.0;

		if (Axis.LowerBound.Mode == EChartAxisBoundMode::Limit) { Lower = Axis.LowerBound.Value; bLower = true; }
		else if (Axis.bUseHardLimits) { Lower = FMath::Min(Axis.HardLimits.X, Axis.HardLimits.Y); bLower = true; }

		if (Axis.UpperBound.Mode == EChartAxisBoundMode::Limit) { Upper = Axis.UpperBound.Value; bUpper = true; }
		else if (Axis.bUseHardLimits) { Upper = FMath::Max(Axis.HardLimits.X, Axis.HardLimits.Y); bUpper = true; }

		if (!bLower && !bUpper)
		{
			return true;
		}

		if (bLower) { Domain.Min = FMath::Max(Domain.Min, Lower); }
		if (bUpper) { Domain.Max = FMath::Min(Domain.Max, Upper); }

		if (Domain.Max - Domain.Min <= KINDA_SMALL_NUMBER)
		{
			if (bLower && bUpper)
			{
				Domain.Min = Lower;
				Domain.Max = Upper;
			}
			else
			{
				return false;
			}
		}

		const double Tolerance = FMath::Max(1e-9, (Upper - Lower) * 1e-9);
		return !(FMath::Abs(Domain.Min - Current.Min) <= Tolerance && FMath::Abs(Domain.Max - Current.Max) <= Tolerance);
	}

	FChartDomain ZoomAround(const FChartDomain& Domain, double AnchorValue, double Factor)
	{
		FChartDomain Result;
		Result.bValid = true;
		Result.Min = AnchorValue + (Domain.Min - AnchorValue) * Factor;
		Result.Max = AnchorValue + (Domain.Max - AnchorValue) * Factor;
		return Result;
	}
}

bool FCartesianZoom::HandleWheel(FChartZoomArgs& Args)
{
	if (!Args.Frame || !Args.Frame->bValid || FMath::IsNearlyZero(Args.WheelDelta))
	{
		return false;
	}

	// 向上滚（delta > 0）= 放大
	const double Factor = FMath::Pow(1.3, -Args.WheelDelta);
	const FVector2D Anchor = Args.Frame->Space.LocalToData(Args.LocalPosition);

	FChartDomain X = ZoomAround(Args.Frame->Space.GetXDomain(), Anchor.X, Factor);
	FChartDomain Y = ZoomAround(Args.Frame->Space.GetYDomain(), Anchor.Y, Factor);

	if (!ClampSide(Args.Frame->Spec.AxisX, Args.Frame->Space.GetXDomain(), X)
		|| !ClampSide(Args.Frame->Spec.AxisY, Args.Frame->Space.GetYDomain(), Y))
	{
		return false;
	}

	Args.Override.bOverrideX = true;
	Args.Override.XRange = FVector2D(X.Min, X.Max);
	Args.Override.bOverrideY = true;
	Args.Override.YRange = FVector2D(Y.Min, Y.Max);
	return true;
}