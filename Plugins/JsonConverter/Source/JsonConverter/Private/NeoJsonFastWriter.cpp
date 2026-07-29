#include "NeoJsonFastWriter.h"

#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/Class.h"
#include "UObject/Field.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UnrealType.h"

namespace
{
	static FCriticalSection GPropertyCacheLock;
	static TMap<const UStruct*, TArray<const FProperty*>> GPropertyCache;

	static bool ShouldSerializeProperty(const FProperty* Property)
	{
		return Property != nullptr
			&& !Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated | CPF_SkipSerialization | CPF_TextExportTransient);
	}

	static FString PropertyToString(const FProperty* Property, const void* ValuePtr)
	{
		FString Output;
		Property->ExportTextItem_Direct(Output, ValuePtr, nullptr, nullptr, PPF_None);
		return Output;
	}

	static FString GetExportPropertyName(const FProperty* Property)
	{
		// 蓝图 Struct 的 GetName() 会带 _Index_GUID 后缀，GetAuthoredName() 才是编辑器里看到的字段名
		return Property->GetAuthoredName();
	}
}

const TArray<const FProperty*>& FNeoJsonFastWriter::GetPropertyCache(const UStruct* Struct)
{
	check(Struct);

	FScopeLock Lock(&GPropertyCacheLock);
	if (const TArray<const FProperty*>* Cached = GPropertyCache.Find(Struct))
	{
		return *Cached;
	}

	TArray<const FProperty*>& Cached = GPropertyCache.Add(Struct);
	for (TFieldIterator<FProperty> It(Struct, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (ShouldSerializeProperty(*It))
		{
			Cached.Add(*It);
		}
	}
	return Cached;
}

FString FNeoJsonFastWriter::ToString(const UStruct* Struct, const void* StructPtr)
{
	if (Struct == nullptr || StructPtr == nullptr)
	{
		return FString();
	}

	FString Output;
	Output.Reserve(64 * 1024);

	TSharedRef<FNeoJsonWriter> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);

	Serialize(Struct, StructPtr, Writer.Get());
	Writer->Close();
	return Output;
}

bool FNeoJsonFastWriter::FromString(const UStruct* Struct, void* StructPtr, const FString& JsonString)
{
	if (Struct == nullptr || StructPtr == nullptr || JsonString.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	return FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), const_cast<UStruct*>(Struct), StructPtr, 0, 0);
}

void FNeoJsonFastWriter::Serialize(const UStruct* Struct, const void* StructPtr, FNeoJsonWriter& Writer)
{
	SerializeStructValue(Struct, StructPtr, Writer);
}

void FNeoJsonFastWriter::SerializeStructValue(const UStruct* Struct, const void* StructPtr, FNeoJsonWriter& Writer)
{
	Writer.WriteObjectStart();
	for (const FProperty* Property : GetPropertyCache(Struct))
	{
		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(StructPtr);
		const FString PropertyName = GetExportPropertyName(Property);
		Writer.WriteIdentifierPrefix(*PropertyName);
		SerializeProperty(Property, ValuePtr, Writer);
	}
	Writer.WriteObjectEnd();
}

void FNeoJsonFastWriter::SerializeArrayProperty(const FProperty* Property, const void* ValuePtr, FNeoJsonWriter& Writer)
{
	const FArrayProperty* ArrayProperty = CastFieldChecked<FArrayProperty>(Property);
	FScriptArrayHelper Helper(ArrayProperty, ValuePtr);
	Writer.WriteArrayStart();
	for (int32 Index = 0; Index < Helper.Num(); ++Index)
	{
		SerializeProperty(ArrayProperty->Inner, Helper.GetRawPtr(Index), Writer);
	}
	Writer.WriteArrayEnd();
}

void FNeoJsonFastWriter::SerializeMapProperty(const FProperty* Property, const void* ValuePtr, FNeoJsonWriter& Writer)
{
	const FMapProperty* MapProperty = CastFieldChecked<FMapProperty>(Property);
	FScriptMapHelper Helper(MapProperty, ValuePtr);
	Writer.WriteObjectStart();
	for (int32 Index = 0; Index < Helper.GetMaxIndex(); ++Index)
	{
		if (!Helper.IsValidIndex(Index))
		{
			continue;
		}

		const FString KeyString = PropertyToString(MapProperty->KeyProp, Helper.GetKeyPtr(Index));
		Writer.WriteIdentifierPrefix(*KeyString);
		SerializeProperty(MapProperty->ValueProp, Helper.GetValuePtr(Index), Writer);
	}
	Writer.WriteObjectEnd();
}

void FNeoJsonFastWriter::SerializeProperty(const FProperty* Property, const void* ValuePtr, FNeoJsonWriter& Writer)
{
	if (Property == nullptr || ValuePtr == nullptr)
	{
		Writer.WriteNull();
		return;
	}

	if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
	{
		Writer.WriteValue(BoolProperty->GetPropertyValue(ValuePtr));
		return;
	}

	if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
	{
		if (NumericProperty->IsFloatingPoint())
		{
			Writer.WriteValue(NumericProperty->GetFloatingPointPropertyValue(ValuePtr));
		}
		else
		{
			Writer.WriteValue((int64)NumericProperty->GetSignedIntPropertyValue(ValuePtr));
		}
		return;
	}

	if (const FStrProperty* StrProperty = CastField<FStrProperty>(Property))
	{
		Writer.WriteValue(StrProperty->GetPropertyValue(ValuePtr));
		return;
	}

	if (const FNameProperty* NameProperty = CastField<FNameProperty>(Property))
	{
		Writer.WriteValue(NameProperty->GetPropertyValue(ValuePtr).ToString());
		return;
	}

	if (const FTextProperty* TextProperty = CastField<FTextProperty>(Property))
	{
		Writer.WriteValue(TextProperty->GetPropertyValue(ValuePtr).ToString());
		return;
	}

	if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
	{
		const int64 Value = EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr);
		if (const UEnum* Enum = EnumProperty->GetEnum())
		{
			Writer.WriteValue(Enum->GetNameStringByValue(Value));
		}
		else
		{
			Writer.WriteValue(Value);
		}
		return;
	}

	if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
	{
		if (ByteProperty->Enum != nullptr)
		{
			const uint8 Value = ByteProperty->GetPropertyValue(ValuePtr);
			Writer.WriteValue(ByteProperty->Enum->GetNameStringByValue(Value));
		}
		else
		{
			Writer.WriteValue(ByteProperty->GetPropertyValue(ValuePtr));
		}
		return;
	}

	if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		SerializeStructValue(StructProperty->Struct, ValuePtr, Writer);
		return;
	}

	if (CastField<FArrayProperty>(Property))
	{
		SerializeArrayProperty(Property, ValuePtr, Writer);
		return;
	}

	if (CastField<FMapProperty>(Property))
	{
		SerializeMapProperty(Property, ValuePtr, Writer);
		return;
	}

	if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
	{
		UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(ValuePtr);
		Writer.WriteValue(ObjectValue != nullptr ? ObjectValue->GetPathName() : FString());
		return;
	}

	if (const FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property))
	{
		Writer.WriteValue(PropertyToString(SoftObjectProperty, ValuePtr));
		return;
	}

	if (const FClassProperty* ClassProperty = CastField<FClassProperty>(Property))
	{
		UObject* ClassObject = ClassProperty->GetPropertyValue(ValuePtr).Get();
		UClass* ClassValue = Cast<UClass>(ClassObject);
		Writer.WriteValue(ClassValue != nullptr ? ClassValue->GetPathName() : FString());
		return;
	}

	if (const FSoftClassProperty* SoftClassProperty = CastField<FSoftClassProperty>(Property))
	{
		Writer.WriteValue(PropertyToString(SoftClassProperty, ValuePtr));
		return;
	}

	Writer.WriteValue(PropertyToString(Property, ValuePtr));
}
