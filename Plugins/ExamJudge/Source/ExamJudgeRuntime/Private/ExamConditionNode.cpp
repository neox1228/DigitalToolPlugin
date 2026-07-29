#include "ExamConditionNode.h"

bool UConditionNode::Evaluate(const TMap<FString, float>& Content) const
{
	switch (Type)
	{
	case EConditionNodeType::Single:
		{
			if (SingleCondition.ActionId.IsEmpty())
			{
				return false;
			}

			const float* Actual = Content.Find(SingleCondition.ActionId);
			if (!Actual)
			{
				return false;
			}

			constexpr float Tolerance = KINDA_SMALL_NUMBER;
			switch (SingleCondition.CompareType)
			{
			case EExamJudgeCompareType::Greater:
				return *Actual > SingleCondition.TargetValue;
			case EExamJudgeCompareType::Less:
				return *Actual < SingleCondition.TargetValue;
			case EExamJudgeCompareType::Equal:
				return FMath::IsNearlyEqual(*Actual, SingleCondition.TargetValue, Tolerance);
			case EExamJudgeCompareType::Unequal:
				return !FMath::IsNearlyEqual(*Actual, SingleCondition.TargetValue, Tolerance);
			case EExamJudgeCompareType::GreaterEqual:
				return *Actual >= SingleCondition.TargetValue - Tolerance;
			case EExamJudgeCompareType::LessEqual:
				return *Actual <= SingleCondition.TargetValue + Tolerance;
			default:
				return false;
			}
		}

	case EConditionNodeType::AND:
		if (Children.IsEmpty())
		{
			return false;
		}

		for (const TObjectPtr<UConditionNode>& Child : Children)
		{
			if (!Child || !Child->Evaluate(Content))
			{
				return false;
			}
		}
		return true;

	case EConditionNodeType::OR:
		if (Children.IsEmpty())
		{
			return false;
		}

		for (const TObjectPtr<UConditionNode>& Child : Children)
		{
			if (Child && Child->Evaluate(Content))
			{
				return true;
			}
		}
		return false;

	default:
		return false;
	}
}
