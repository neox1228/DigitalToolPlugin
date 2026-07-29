#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "MessageSchemaValidator.generated.h"

UCLASS()
class MESSAGEBUSEDITOR_API UMessageSchemaValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

public:
	// 只校验 MessageSchemaAsset，避免影响项目中的其他 DataAsset。
	virtual bool CanValidateAsset_Implementation(UObject* InAsset) const override;

	// 编辑器数据校验入口：检查空 Topic、空 PayloadStruct、重复 Topic。
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(UObject* InAsset, TArray<FText>& ValidationErrors) override;
};
