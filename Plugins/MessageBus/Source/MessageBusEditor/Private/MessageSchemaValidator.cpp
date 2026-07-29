#include "MessageSchemaValidator.h"

#include "MessageSchemaAsset.h"

bool UMessageSchemaValidator::CanValidateAsset_Implementation(UObject* InAsset) const
{
	return InAsset && InAsset->IsA<UMessageSchemaAsset>();
}

EDataValidationResult UMessageSchemaValidator::ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors)
{
	const UMessageSchemaAsset* Schema = Cast<UMessageSchemaAsset>(InAsset);
	if (!Schema)
	{
		return EDataValidationResult::NotValidated;
	}

	EDataValidationResult Result = EDataValidationResult::Valid;

	// 用集合记录已经出现过的 Topic，用于发现重复配置。
	TSet<FName> SeenTopics;

	for (int32 Index = 0; Index < Schema->Topics.Num(); ++Index)
	{
		const FMessageSchemaItem& Item = Schema->Topics[Index];

		// Topic 为空时无法作为消息路由键。
		if (Item.Topic.IsNone())
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::FromString(FString::Printf(TEXT("Topics[%d]: Topic is None."), Index)));
			continue;
		}

		// 没有 PayloadStruct 时，运行时无法做强类型校验。
		if (!Item.PayloadStruct)
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::FromString(FString::Printf(TEXT("Topics[%d] '%s': PayloadStruct is null."), Index, *Item.Topic.ToString())));
			continue;
		}

		// 重复 Topic 会导致协议含义不唯一，运行时只会保留一个映射，因此必须报错。
		if (SeenTopics.Contains(Item.Topic))
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::FromString(FString::Printf(TEXT("Duplicate topic '%s' (Topics[%d])."), *Item.Topic.ToString(), Index)));
		}
		else
		{
			SeenTopics.Add(Item.Topic);
		}
	}

	return Result;
}
