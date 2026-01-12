// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Misc/Guid.h"
#include "UObject/Class.h"
#include "ChartWebJSFunction.generated.h"

class FWebJSScripting;

struct CHARTWEBBROWSER_API FChartWebJSParam
{

	struct IStructWrapper
	{
		virtual ~IStructWrapper() {};
		virtual UStruct* GetTypeInfo() = 0;
		virtual const void* GetData() = 0;
		virtual IStructWrapper* Clone() = 0;
	};

	template <typename T> struct FStructWrapper
		: public IStructWrapper
	{
		T StructValue;
		FStructWrapper(const T& InValue)
			: StructValue(InValue)
		{}
		virtual ~FStructWrapper()
		{}
		virtual UStruct* GetTypeInfo() override
		{
			return T::StaticStruct();
		}
		virtual const void* GetData() override
		{
			return &StructValue;
		}
		virtual IStructWrapper* Clone() override
		{
			return new FStructWrapper<T>(StructValue);
		}
	};

	FChartWebJSParam() : Tag(PTYPE_NULL) {}
	FChartWebJSParam(bool Value) : Tag(PTYPE_BOOL), BoolValue(Value) {}
	FChartWebJSParam(int8 Value) : Tag(PTYPE_INT), IntValue(Value) {}
	FChartWebJSParam(int16 Value) : Tag(PTYPE_INT), IntValue(Value) {}
	FChartWebJSParam(int32 Value) : Tag(PTYPE_INT), IntValue(Value) {}
	FChartWebJSParam(uint8 Value) : Tag(PTYPE_INT), IntValue(Value) {}
	FChartWebJSParam(uint16 Value) : Tag(PTYPE_INT), IntValue(Value) {}
	FChartWebJSParam(uint32 Value) : Tag(PTYPE_DOUBLE), DoubleValue(Value) {}
	FChartWebJSParam(int64 Value) : Tag(PTYPE_DOUBLE), DoubleValue(Value) {}
	FChartWebJSParam(uint64 Value) : Tag(PTYPE_DOUBLE), DoubleValue(Value) {}
	FChartWebJSParam(double Value) : Tag(PTYPE_DOUBLE), DoubleValue(Value) {}
	FChartWebJSParam(float Value) : Tag(PTYPE_DOUBLE), DoubleValue(Value) {}
	FChartWebJSParam(const FString& Value) : Tag(PTYPE_STRING), StringValue(new FString(Value)) {}
	FChartWebJSParam(const FText& Value) : Tag(PTYPE_STRING), StringValue(new FString(Value.ToString())) {}
	FChartWebJSParam(const FName& Value) : Tag(PTYPE_STRING), StringValue(new FString(Value.ToString())) {}
	FChartWebJSParam(const TCHAR* Value) : Tag(PTYPE_STRING), StringValue(new FString(Value)) {}
	FChartWebJSParam(UObject* Value) : Tag(PTYPE_OBJECT), ObjectValue(Value) {}
	template <typename T> FChartWebJSParam(const T& Value,
		typename TEnableIf<!TIsPointer<T>::Value, UStruct>::Type* InTypeInfo=T::StaticStruct())
		: Tag(PTYPE_STRUCT)
		, StructValue(new FStructWrapper<T>(Value))
	{}
	template <typename T> FChartWebJSParam(const TArray<T>& Value)
		: Tag(PTYPE_ARRAY)
	{
		ArrayValue = new TArray<FChartWebJSParam>();
		ArrayValue->Reserve(Value.Num());
		for(T Item : Value)
		{
			ArrayValue->Add(FChartWebJSParam(Item));
		}
	}
	template <typename T> FChartWebJSParam(const TMap<FString, T>& Value)
		: Tag(PTYPE_MAP)
	{
		MapValue = new TMap<FString, FChartWebJSParam>();
		MapValue->Reserve(Value.Num());
		for(const auto& Pair : Value)
		{
			MapValue->Add(Pair.Key, FChartWebJSParam(Pair.Value));
		}
	}
	template <typename K, typename T> FChartWebJSParam(const TMap<K, T>& Value)
		: Tag(PTYPE_MAP)
	{
		MapValue = new TMap<FString, FChartWebJSParam>();
		MapValue->Reserve(Value.Num());
		for(const auto& Pair : Value)
		{
			MapValue->Add(Pair.Key.ToString(), FChartWebJSParam(Pair.Value));
		}
	}
	FChartWebJSParam(const FChartWebJSParam& Other);
	FChartWebJSParam(FChartWebJSParam&& Other);
	~FChartWebJSParam();

	enum { PTYPE_NULL, PTYPE_BOOL, PTYPE_INT, PTYPE_DOUBLE, PTYPE_STRING, PTYPE_OBJECT, PTYPE_STRUCT, PTYPE_ARRAY, PTYPE_MAP } Tag;
	union
	{
		bool BoolValue;
		double DoubleValue;
		int32 IntValue;
		UObject* ObjectValue;
		const FString* StringValue;
		IStructWrapper* StructValue;
		TArray<FChartWebJSParam>* ArrayValue;
		TMap<FString, FChartWebJSParam>* MapValue;
	};

};

class FWebJSScripting;

/** Base class for JS callback objects. */
USTRUCT()
struct CHARTWEBBROWSER_API FChartWebJSCallbackBase
{
	GENERATED_USTRUCT_BODY()
	FChartWebJSCallbackBase()
	{}

	bool IsValid() const
	{
		return ScriptingPtr.IsValid();
	}


protected:
	FChartWebJSCallbackBase(TSharedPtr<FWebJSScripting> InScripting, const FGuid& InCallbackId)
		: ScriptingPtr(InScripting)
		, CallbackId(InCallbackId)
	{}

	void Invoke(int32 ArgCount, FChartWebJSParam Arguments[], bool bIsError = false) const;

private:

	TWeakPtr<FWebJSScripting> ScriptingPtr;
	FGuid CallbackId;
};


/**
 * Representation of a remote JS function.
 * FWebJSFunction objects represent a JS function and allow calling them from native code.
 * FWebJSFunction objects can also be added to delegates and events using the Bind/AddLambda method.
 */
USTRUCT()
struct CHARTWEBBROWSER_API FChartWebJSFunction
	: public FChartWebJSCallbackBase
{
	GENERATED_USTRUCT_BODY()

	FChartWebJSFunction()
		: FChartWebJSCallbackBase()
	{}

	FChartWebJSFunction(TSharedPtr<FWebJSScripting> InScripting, const FGuid& InFunctionId)
		: FChartWebJSCallbackBase(InScripting, InFunctionId)
	{}

	template<typename ...ArgTypes> void operator()(ArgTypes... Args) const
	{
		FChartWebJSParam ArgArray[sizeof...(Args)] = {FChartWebJSParam(Args)...};
		Invoke(sizeof...(Args), ArgArray);
	}
};

/** 
 *  Representation of a remote JS async response object.
 *  UFUNCTIONs taking a FWebJSResponse will get it passed in automatically when called from a web browser.
 *  Pass a result or error back by invoking Success or Failure on the object.
 *  UFunctions accepting a FWebJSResponse should have a void return type, as any value returned from the function will be ignored.
 *  Calling the response methods does not have to happen before returning from the function, which means you can use this to implement asynchronous functionality.
 *
 *  Note that the remote object will become invalid as soon as a result has been delivered, so you can only call either Success or Failure once.
 */
USTRUCT()
struct CHARTWEBBROWSER_API FChartWebJSResponse
	: public FChartWebJSCallbackBase
{
	GENERATED_USTRUCT_BODY()

	FChartWebJSResponse()
		: FChartWebJSCallbackBase()
	{}

	FChartWebJSResponse(TSharedPtr<FWebJSScripting> InScripting, const FGuid& InCallbackId)
		: FChartWebJSCallbackBase(InScripting, InCallbackId)
	{}

	/**
	 * Indicate successful completion without a return value.
	 * The remote Promise's then() handler will be executed without arguments.
	 */
	void Success() const
	{
		Invoke(0, nullptr, false);
	}

	/**
	 * Indicate successful completion passing a return value back.
	 * The remote Promise's then() handler will be executed with the value passed as its single argument.
	 */
	template<typename T>
	void Success(T Arg) const
	{
		FChartWebJSParam ArgArray[1] = {FChartWebJSParam(Arg)};
		Invoke(1, ArgArray, false);
	}

	/**
	 * Indicate failed completion, passing an error message back to JS.
	 * The remote Promise's catch() handler will be executed with the value passed as the error reason.
	 */
	template<typename T>
	void Failure(T Arg) const
	{
		FChartWebJSParam ArgArray[1] = {FChartWebJSParam(Arg)};
		Invoke(1, ArgArray, true);
	}


};
