#pragma once

#include "CoreMinimal.h"

#include "ExamJudgeTypes.generated.h"

UENUM(BlueprintType)
enum class EExamJudgeCompareType : uint8
{
	Greater		UMETA(DisplayName = ">"),
	Less		UMETA(DisplayName = "<"),
	Equal		UMETA(DisplayName = "=="),
	GreaterEqual	UMETA(DisplayName = ">="),
	Unequal		UMETA(DisplayName = "!="),
	LessEqual	UMETA(DisplayName = "<=")
};

USTRUCT(BlueprintType)
struct EXAMJUDGERUNTIME_API FSingleCondition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FString ActionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	EExamJudgeCompareType CompareType = EExamJudgeCompareType::Equal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float TargetValue = 0.0f;
};
