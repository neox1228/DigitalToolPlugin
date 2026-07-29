// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonBlueprintLibrary.generated.h"

/**
 * JSON 序列化/反序列化蓝图函数库
 */
UCLASS()
class JSONCONVERTER_API UJsonBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	/**
	 * 将传入的数据结构序列化为 JSON 字符串
	 * @param Data 任意 Struct 变量（可包含多个字段、TArray、嵌套 Struct 等）
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "JsonConverter | Library", meta = (CustomStructureParam = "Data", DisplayName = "To Json String"))
	static FString ToJsonString(const int32& Data);
	DECLARE_FUNCTION(execToJsonString);

	/**
	 * 将 JSON 字符串写入 Saved/Json 目录下的文件
	 * @param JsonString 要保存的 JSON 字符串
	 * @param FileName JSON 文件名（可省略 .json 后缀）
	 * @return 是否写入成功
	 */
	UFUNCTION(BlueprintCallable, Category = "JsonConverter | Library", meta = (AutoCreateRefTerm = "FileName", DisplayName = "Save Json String To File"))
	static bool SaveJsonStringToFile(const FString& JsonString, const FString& FileName);

	/**
	 * 将传入的数据结构序列化并写入 Saved/Json 目录下的 JSON 文件
	 * @param Data 任意 Struct 变量（可包含多个字段、TArray、嵌套 Struct 等）
	 * @param FileName JSON 文件名（可省略 .json 后缀）
	 * @return 是否写入成功
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "JsonConverter | Library", meta = (CustomStructureParam = "Data", AutoCreateRefTerm = "FileName", DisplayName = "Save Struct To Json File"))
	static bool SaveStructToJsonFile(const int32& Data, const FString& FileName);
	DECLARE_FUNCTION(execSaveStructToJsonFile);

	/**
	 * 读取 Saved/Json 目录下的 JSON 文件内容
	 * @param FileName JSON 文件名（可省略 .json 后缀）
	 * @return 文件中的 JSON 字符串；读取失败时返回空字符串
	 */
	UFUNCTION(BlueprintCallable, Category = "JsonConverter | Library", meta = (AutoCreateRefTerm = "FileName", DisplayName = "Load Json File To String"))
	static FString LoadJsonFileToString(const FString& FileName);

	/**
	 * 根据传入的数据结构类型解析 JSON，并写入该变量；每次执行会先清空变量原有数据
	 * @param JsonString JSON 字符串
	 * @param OutData 任意 Struct 变量，解析结果会写入这里
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "JsonConverter | Library", meta = (CustomStructureParam = "OutData", DisplayName = "From Json String"))
	static bool FromJsonString(const FString& JsonString, int32& OutData);
	DECLARE_FUNCTION(execFromJsonString);
};
