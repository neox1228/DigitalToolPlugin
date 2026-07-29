#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GraphDocument.h"
#include "SGraphDesignerCanvas.h"
#include "GraphCanvasWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGraphDesignerConnectionCreatedEvent, const FString&, StartPinId, const FString&, EndPinId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGraphDesignerConnectionCreatedByNameEvent, const FString&, StartPinName, const FString&, EndPinName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGraphDesignerSelectionChangedEvent, const TArray<FString>&, SelectedElementIds);

UCLASS(BlueprintType, Blueprintable)
class GRAPHDESIGNER_API UGraphCanvasWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGraphCanvasWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY(BlueprintAssignable, Category = "GraphDesigner|Events")
	FGraphDesignerConnectionCreatedEvent OnConnectionCreated;

	UPROPERTY(BlueprintAssignable, Category = "GraphDesigner|Events")
	FGraphDesignerConnectionCreatedByNameEvent OnConnectionCreatedByName;

	UPROPERTY(BlueprintAssignable, Category = "GraphDesigner|Events")
	FGraphDesignerSelectionChangedEvent OnSelectionChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Appearance")
	FLinearColor BackgroundColor = FLinearColor(0.04f, 0.045f, 0.055f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Appearance")
	FLinearColor GridColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.08f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Appearance")
	FLinearColor SelectionColor = FLinearColor(0.1f, 0.45f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Display")
	FString SavedGraphName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Display")
	bool bLoadSavedGraphOnConstruct = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Interaction")
	bool bGraphEditingLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Connections")
	EGraphDesignerConnectionRoutingMode ConnectionRoutingMode = EGraphDesignerConnectionRoutingMode::AutoOrthogonal;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	UGraphDocument* GetDocument() const { return Document; }

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	void SetDocument(UGraphDocument* InDocument);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	void CreateNewGraph();

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	FString AddElement(EGraphDesignerElementType Type, FVector2D Position, FVector2D Size);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	FString AddStyledElement(EGraphDesignerElementType Type, FVector2D Position, FVector2D Size, FLinearColor FillColor, FLinearColor BorderColor);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	bool SetElementText(const FString& ElementId, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	bool SetElementImagePath(const FString& ElementId, const FString& ImagePath);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	bool SetElementTransform(const FString& ElementId, FVector2D Position, FVector2D Size, float Rotation);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Pins")
	FString AddPinToElement(const FString& ElementId, FVector2D RelativePosition, const FString& PinText = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Connections")
	FString AddConnection(const FString& StartPinId, const FString& EndPinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Connections")
	bool DisconnectConnectionByPins(const FString& StartPinId, const FString& EndPinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Connections")
	bool SetConnectionLineStyle(const FString& ConnectionId, EGraphDesignerConnectionLineStyle LineStyle);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	void SetRequiredConnections(const TArray<FGraphDesignerRequiredConnection>& InRequiredConnections);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	TArray<FGraphDesignerRequiredConnection> GetRequiredConnections() const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	int32 AddRequiredConnection(const FString& StartPinId, const FString& EndPinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	bool UpdateRequiredConnection(int32 Index, const FString& StartPinId, const FString& EndPinId);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	bool RemoveRequiredConnectionAt(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	bool IsRequiredConnection(const FString& StartPinId, const FString& EndPinId) const;
	
	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	bool IsRequiredConnectionUnDirected(const FString& StartPinId, const FString& EndPinId, FString& AnswerStartPinId, FString& AnswerEndPinId) const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Connections")
	void SetConnectionRoutingMode(EGraphDesignerConnectionRoutingMode InRoutingMode);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Connections")
	EGraphDesignerConnectionRoutingMode GetConnectionRoutingMode() const { return ConnectionRoutingMode; }

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Selection")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Interaction")
	void SetGraphEditingLocked(bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Interaction")
	bool IsGraphEditingLocked() const { return bGraphEditingLocked; }

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Viewport")
	void SetZoom(float InZoom);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Viewport")
	float GetZoom() const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Viewport")
	void SetCanvasPan(FVector2D InPan);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Viewport")
	FVector2D GetCanvasPan() const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Serialization")
	bool SaveGraph(const FString& GraphName);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Serialization", meta = (DeprecatedFunction, DeprecationMessage = "Use LoadSavedGraphByName on GraphCanvasWidget for runtime display."))
	bool LoadGraph(const FString& GraphName);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Display")
	bool LoadSavedGraphByName(const FString& GraphName);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	UGraphDocument* Document = nullptr;

	TSharedPtr<SGraphDesignerCanvas> GraphCanvas;

	void EnsureDocument();
	void InvalidateCanvas();
	FString ResolvePinName(const FString& PinId) const;
};
