#pragma once

#include "CoreMinimal.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonWriter.h"

class FJsonObject;
class FJsonValue;
class FProperty;
class UStruct;
class UObject;

using FNeoJsonWriter = TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>;

class JSONCONVERTER_API FNeoJsonFastWriter
{
public:
	static FString ToString(const UStruct* Struct, const void* StructPtr);
	static bool FromString(const UStruct* Struct, void* StructPtr, const FString& JsonString);

	static void Serialize(const UStruct* Struct, const void* StructPtr, FNeoJsonWriter& Writer);
	static void SerializeProperty(const FProperty* Property, const void* ValuePtr, FNeoJsonWriter& Writer);

private:
	static const TArray<const FProperty*>& GetPropertyCache(const UStruct* Struct);
	static void SerializeStructValue(const UStruct* Struct, const void* StructPtr, FNeoJsonWriter& Writer);
	static void SerializeArrayProperty(const FProperty* Property, const void* ValuePtr, FNeoJsonWriter& Writer);
	static void SerializeMapProperty(const FProperty* Property, const void* ValuePtr, FNeoJsonWriter& Writer);
};
