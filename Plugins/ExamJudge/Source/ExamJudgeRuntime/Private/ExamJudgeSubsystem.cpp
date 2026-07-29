#include "ExamJudgeSubsystem.h"

#include "ExamConditionNode.h"
#include "ExamJudgeDataAsset.h"

void UExamJudgeSubsystem::SetCurrentSubQuestion(const FString& SubQuestionId)
{
	CurrentSubQuestionId = SubQuestionId;
	Content.Empty();
}

bool UExamJudgeSubsystem::SubmitAction(const FString& ActionId, float Value)
{
	if (!ActionId.IsEmpty())
	{
		Content.Add(ActionId, Value);
	}

	return Evaluate();
}

void UExamJudgeSubsystem::SetJudgeDataAsset(UExamJudgeDataAsset* InJudgeDataAsset)
{
	JudgeDataAsset = InJudgeDataAsset;
}

bool UExamJudgeSubsystem::Evaluate() const
{
	if (CurrentSubQuestionId.IsEmpty())
	{
		return false;
	}

	if (JudgeDataAsset)
	{
		const UConditionNode* RootNode = JudgeDataAsset->FindConditionRoot(CurrentSubQuestionId);
		if (RootNode)
		{
			return RootNode->Evaluate(Content);
		}
	}

	const TObjectPtr<UConditionNode>* RootNode = ASTMap.Find(CurrentSubQuestionId);
	return RootNode && RootNode->Get() && RootNode->Get()->Evaluate(Content);
}
