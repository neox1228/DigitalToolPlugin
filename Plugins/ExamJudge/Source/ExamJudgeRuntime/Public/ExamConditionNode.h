#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "ExamJudgeTypes.h"
#include "ExamConditionNode.generated.h"

UENUM(BlueprintType)
enum class EConditionNodeType : uint8
{
	Single,
	AND,
	OR
};

/** 条件 AST 节点：Single / AND / OR。 */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class EXAMJUDGERUNTIME_API UConditionNode : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	EConditionNodeType Type = EConditionNodeType::Single;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (EditCondition = "Type == EConditionNodeType::Single", EditConditionHides))
	FSingleCondition SingleCondition;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Condition", meta = (EditCondition = "Type != EConditionNodeType::Single", EditConditionHides))
	TArray<TObjectPtr<UConditionNode>> Children;

	bool Evaluate(const TMap<FString, float>& Content) const;
};
