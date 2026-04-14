#include "NCharts/Public/Core/NChartMathLib.h"

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
		// 使用 UE 原生排序，传入 Lambda比较器
		InPoints.Sort([](const FVector2D& A, const FVector2D& B)
		{
			return A.X < B.X; // 升序 ，按照 X 来排
		});
	}
	else
	{
		InPoints.Sort([](const FVector2D& A, const FVector2D& B)
		{
			return A.X > B.Y; // 降序
		});
	}
}
