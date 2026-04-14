#pragma once

class NChartMathLib
{
public:
	NChartMathLib();
	~NChartMathLib();
	/**
	 * 对传入的点位数据按 X 轴进行排序
	 * @param InPoints 点位数据集
	 * @param bAscending true:升序；false:降序
	 */
	static void SortPointsByX(UPARAM(ref) TArray<FVector2D>& InPoints, bool bAscending = true);
	
};
