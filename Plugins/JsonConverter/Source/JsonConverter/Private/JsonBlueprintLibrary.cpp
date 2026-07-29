#include "JsonBlueprintLibrary.h"

#include "NeoJsonFastWriter.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"

namespace
{
	FString ResolveJsonFilePath(const FString& FileName)
	{
		FString NormalizedName = FileName.TrimStartAndEnd();
		if (NormalizedName.IsEmpty())
		{
			return FString();
		}

		if (!NormalizedName.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase))
		{
			NormalizedName += TEXT(".json");
		}

		return FPaths::Combine(FPaths::ProjectContentDir(),TEXT("HuaYang"), TEXT("GraphsJson"), NormalizedName);
	}

	bool EnsureJsonDirectoryExists()
	{
		const FString JsonDirectory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Json"));
		return IFileManager::Get().MakeDirectory(*JsonDirectory, true);
	}

	void ResetStructToDefaults(UScriptStruct* Struct, void* StructPtr)
	{
		if (Struct == nullptr || StructPtr == nullptr)
		{
			return;
		}

		const int32 StructSize = Struct->GetStructureSize();
		TArray<uint8> DefaultBuffer;
		DefaultBuffer.SetNumZeroed(StructSize);
		Struct->InitializeStruct(DefaultBuffer.GetData());
		Struct->CopyScriptStruct(StructPtr, DefaultBuffer.GetData());
		Struct->DestroyStruct(DefaultBuffer.GetData());
	}

	bool WriteJsonStringToFile(const FString& JsonString, const FString& FileName)
	{
		if (JsonString.IsEmpty() || FileName.IsEmpty())
		{
			return false;
		}

		if (!EnsureJsonDirectoryExists())
		{
			return false;
		}

		const FString FilePath = ResolveJsonFilePath(FileName);
		return FFileHelper::SaveStringToFile(JsonString, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

FString UJsonBlueprintLibrary::ToJsonString(const int32& Data)
{
	// CustomThunk：蓝图调用 execToJsonString，不会进入此函数体。
	ensureMsgf(false, TEXT("ToJsonString should only be called from Blueprint (execToJsonString)."));
	return FString();
}

DEFINE_FUNCTION(UJsonBlueprintLibrary::execToJsonString)
{
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const void* StructPtr = Stack.MostRecentPropertyAddress;
	const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	FString JsonString;
	if (StructProp && StructPtr)
	{
		JsonString = FNeoJsonFastWriter::ToString(StructProp->Struct, StructPtr);
	}

	*(FString*)RESULT_PARAM = JsonString;
}

bool UJsonBlueprintLibrary::SaveJsonStringToFile(const FString& JsonString, const FString& FileName)
{
	return WriteJsonStringToFile(JsonString, FileName);
}

bool UJsonBlueprintLibrary::SaveStructToJsonFile(const int32& Data, const FString& FileName)
{
	// CustomThunk：蓝图调用 execSaveStructToJsonFile，不会进入此函数体。
	ensureMsgf(false, TEXT("SaveStructToJsonFile should only be called from Blueprint (execSaveStructToJsonFile)."));
	return false;
}

DEFINE_FUNCTION(UJsonBlueprintLibrary::execSaveStructToJsonFile)
{
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	const void* StructPtr = Stack.MostRecentPropertyAddress;
	const FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_GET_PROPERTY(FStrProperty, FileName);

	P_FINISH;

	bool bResult = false;
	if (StructProp && StructPtr && !FileName.IsEmpty())
	{
		const FString JsonString = FNeoJsonFastWriter::ToString(StructProp->Struct, StructPtr);
		bResult = WriteJsonStringToFile(JsonString, FileName);
	}

	*(bool*)RESULT_PARAM = bResult;
}

FString UJsonBlueprintLibrary::LoadJsonFileToString(const FString& FileName)
{
	if (FileName.IsEmpty())
	{
		return FString();
	}

	const FString FilePath = ResolveJsonFilePath(FileName);
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return FString();
	}

	return JsonString;
}

bool UJsonBlueprintLibrary::FromJsonString(const FString& JsonString, int32& OutData)
{
	// CustomThunk：蓝图调用 execFromJsonString，不会进入此函数体。
	ensureMsgf(false, TEXT("FromJsonString should only be called from Blueprint (execFromJsonString)."));
	return false;
}

DEFINE_FUNCTION(UJsonBlueprintLibrary::execFromJsonString)
{
	P_GET_PROPERTY(FStrProperty, JsonString);

	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);

	void* StructPtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	bool bResult = false;
	if (StructProp && StructPtr)
	{
		ResetStructToDefaults(StructProp->Struct, StructPtr);

		if (!JsonString.IsEmpty())
		{
			bResult = FNeoJsonFastWriter::FromString(StructProp->Struct, StructPtr, JsonString);
		}
	}

	*(bool*)RESULT_PARAM = bResult;
}
