#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ExamJudgeSubsystem.generated.h"

class UConditionNode;
class UExamJudgeDataAsset;

/**
 * 极简实时小题完成判定引擎：Content + ASTMap + Evaluate。
 * 不记录历史，不回放操作，只用当前状态表重算当前小题规则。
 */
UCLASS()
class EXAMJUDGERUNTIME_API UExamJudgeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 切换当前小题：只切换 SubQuestionId，并清空上一题 Content。 */
	UFUNCTION(BlueprintCallable, Category = "ExamJudge")
	void SetCurrentSubQuestion(const FString& SubQuestionId);

	/** 更新 Content[ActionId]，立即按当前 SubQuestionId 对应 AST 重新判定。 */
	UFUNCTION(BlueprintCallable, Category = "ExamJudge")
	bool SubmitAction(const FString& ActionId, float Value);

	UFUNCTION(BlueprintCallable, Category = "ExamJudge")
	void SetJudgeDataAsset(UExamJudgeDataAsset* InJudgeDataAsset);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExamJudge")
	TObjectPtr<UExamJudgeDataAsset> JudgeDataAsset;

	/** SubQuestionId -> ConditionAST。 */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "ExamJudge")
	TMap<FString, TObjectPtr<UConditionNode>> ASTMap;

private:
	FString CurrentSubQuestionId;

	/** ActionId -> 当前最新值。 */
	TMap<FString, float> Content;

	bool Evaluate() const;
};
