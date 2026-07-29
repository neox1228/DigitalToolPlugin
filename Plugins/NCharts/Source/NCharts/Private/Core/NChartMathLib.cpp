// Copyright NCharts Plugin. All Rights Reserved.
// NChartMathLib 实现：图表数据预处理工具

#include "Core/NChartMathLib.h"

NChartMathLib::NChartMathLib()
{
}

NChartMathLib::~NChartMathLib()
{
}

void NChartMathLib::SortPointsByX(TArray<FVector2D>& InPoints, bool bAscending)
{
	if (bAscending)
	{
		// 升序：按 X 坐标从小到大排列
		InPoints.Sort([](const FVector2D& A, const FVector2D& B)
		{
			return A.X < B.X;
		});
	}
	else
	{
		// 降序：按 X 坐标从大到小排列
		InPoints.Sort([](const FVector2D& A, const FVector2D& B)
		{
			return A.X > B.Y;
		});
	}
}
