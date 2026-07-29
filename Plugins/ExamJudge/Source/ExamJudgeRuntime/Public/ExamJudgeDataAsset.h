#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ExamConditionNode.h"
#include "ExamJudgeDataAsset.generated.h"

/** Exam rule asset: SubQuestionId -> condition AST. */
UCLASS(BlueprintType)
class EXAMJUDGERUNTIME_API UExamJudgeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Each sub-question owns one root condition node. */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "ExamJudge")
	TMap<FString, TObjectPtr<UConditionNode>> ASTMap;

	const UConditionNode* FindConditionRoot(const FString& SubQuestionId) const;
};
