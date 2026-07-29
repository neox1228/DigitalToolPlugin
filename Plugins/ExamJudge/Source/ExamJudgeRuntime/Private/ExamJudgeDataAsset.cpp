#include "ExamJudgeDataAsset.h"

const UConditionNode* UExamJudgeDataAsset::FindConditionRoot(const FString& SubQuestionId) const
{
	if (SubQuestionId.IsEmpty())
	{
		return nullptr;
	}

	const TObjectPtr<UConditionNode>* RootNode = ASTMap.Find(SubQuestionId);
	return RootNode ? RootNode->Get() : nullptr;
}
