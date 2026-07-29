#pragma once

#include "CoreMinimal.h"
#include "GraphDesignerTypes.h"
#include "UObject/Object.h"
#include "GraphDocument.generated.h"

UCLASS(BlueprintType)
class GRAPHDESIGNER_API UGraphDocument : public UObject
{
	GENERATED_BODY()

public:
	UGraphDocument();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString DocumentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FGraphDesignerElement> Elements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FGraphDesignerConnection> Connections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FGraphDesignerRequiredConnection> RequiredConnections;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	void Clear();

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	FString AddElement(EGraphDesignerElementType Type, FVector2D Position, FVector2D Size);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool RemoveElement(const FString& ElementId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	int32 RemoveElements(const TArray<FString>& ElementIds);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool UpdateElement(const FGraphDesignerElement& Element);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool SetElementSelected(const FString& ElementId, bool bSelected);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	FString AddPinToElement(const FString& ElementId, FVector2D RelativePosition, const FString& PinText = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool RemovePin(const FString& PinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool UpdatePin(const FGraphDesignerPin& Pin);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool ArrangePinsOnSameSide(const FString& ElementId, FVector2D SidePosition);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	FString AddConnection(const FString& StartPinId, const FString& EndPinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	FString AddConnectionWithRoutePoints(const FString& StartPinId, const FString& EndPinId, const TArray<FVector2D>& RoutePoints);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool RemoveConnection(const FString& ConnectionId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool RemoveConnectionBetweenPins(const FString& StartPinId, const FString& EndPinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool UpdateConnection(const FGraphDesignerConnection& Connection);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool SetConnectionLineStyle(const FString& ConnectionId, EGraphDesignerConnectionLineStyle LineStyle);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	void SetRequiredConnections(const TArray<FGraphDesignerRequiredConnection>& InRequiredConnections);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	TArray<FGraphDesignerRequiredConnection> GetRequiredConnections() const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	int32 AddRequiredConnection(const FString& StartPinId, const FString& EndPinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool UpdateRequiredConnection(int32 Index, const FString& StartPinId, const FString& EndPinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool RemoveRequiredConnectionAt(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool IsRequiredConnection(const FString& StartPinId, const FString& EndPinId) const;
	
	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool IsRequiredConnectionUndirected(const FString& StartPinId, const FString& EndPinId, FString& AnswerStartPinId, FString& AnswerEndPinId) const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	TArray<FGraphDesignerElement> GetElements() const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	TArray<FGraphDesignerConnection> GetConnections() const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool SaveToJsonString(FString& OutJson) const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool LoadFromJsonString(const FString& Json);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool SaveToFile(const FString& GraphName) const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	bool LoadFromFile(const FString& GraphName);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	FString GetLastError() const { return LastError; }

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	static FString GetDefaultGraphDirectory();

	/** Converts an absolute or relative image path to a path relative to ProjectContentDir for serialization. */
	static FString MakeRelativeImagePath(const FString& Path);

	/** Resolves a stored image path (relative or absolute) to an absolute filesystem path. */
	static FString ResolveImagePath(const FString& StoredPath);

	FGraphDesignerElement* FindElement(const FString& ElementId);
	const FGraphDesignerElement* FindElement(const FString& ElementId) const;
	FGraphDesignerConnection* FindConnection(const FString& ConnectionId);
	const FGraphDesignerConnection* FindConnection(const FString& ConnectionId) const;
	FGraphDesignerPin* FindPin(const FString& PinId, FGraphDesignerElement** OutOwnerElement = nullptr);
	const FGraphDesignerPin* FindPin(const FString& PinId, const FGraphDesignerElement** OutOwnerElement = nullptr) const;
	bool GetPinWorldPosition(const FString& PinId, FVector2D& OutWorldPosition) const;
	void RefreshElementPinIds(const FString& ElementId);

private:
	UPROPERTY(Transient)
	FString LastError;

	static FString MakeId(const TCHAR* Prefix);
	static FString NormalizeGraphFileName(const FString& GraphName);
	static FString NormalizePinIdPart(const FString& Text, const FString& Fallback);
	FString MakePinId(const FGraphDesignerElement& OwnerElement, const FGraphDesignerPin& Pin) const;
	FString MakeUniquePinId(const FString& BasePinId, const FString& CurrentPinId = TEXT("")) const;
	void RefreshPinId(FGraphDesignerElement& OwnerElement, FGraphDesignerPin& Pin);
	void RefreshAllPinIds();
	void ReplacePinReferences(const FString& OldPinId, const FString& NewPinId);
	bool PinExists(const FString& PinId) const;
};
