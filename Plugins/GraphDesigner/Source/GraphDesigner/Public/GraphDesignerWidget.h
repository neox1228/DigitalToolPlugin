#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GraphDocument.h"
#include "SGraphDesignerCanvas.h"
#include "GraphDesignerWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGraphDesignerWidgetSelectionChangedEvent, const TArray<FString>&, SelectedElementIds);

class SGraphDesignerPalette;
class SGraphDesignerInspector;
class SGraphDesignerAnswerPanel;
class SEditableTextBox;

UCLASS(BlueprintType, Blueprintable)
class GRAPHDESIGNER_API UGraphDesignerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGraphDesignerWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UPROPERTY(BlueprintAssignable, Category = "GraphDesigner|Events")
	FGraphDesignerWidgetSelectionChangedEvent OnSelectionChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Palette")
	TArray<FGraphDesignerNodeTemplate> NodeTemplates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Images")
	bool bAutoLoadRuntimeImages = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Palette", meta = (ClampMin = "120.0"))
	float PaletteWidth = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Inspector", meta = (ClampMin = "160.0"))
	float InspectorWidth = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Appearance")
	FLinearColor BackgroundColor = FLinearColor(0.04f, 0.045f, 0.055f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Appearance")
	FLinearColor GridColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.08f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Appearance")
	FLinearColor SelectionColor = FLinearColor(0.1f, 0.45f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner|Connections")
	EGraphDesignerConnectionRoutingMode ConnectionRoutingMode = EGraphDesignerConnectionRoutingMode::ManualRoutePoints;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	UGraphDocument* GetDocument() const { return Document; }

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	void SetDocument(UGraphDocument* InDocument);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Document")
	void CreateNewGraph();

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Palette")
	void SetNodeTemplates(const TArray<FGraphDesignerNodeTemplate>& InNodeTemplates);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Palette")
	void ResetToDefaultTemplates();

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Images")
	void LoadRuntimeImageTemplatesFromDirectory(const FString& Directory, bool bRecursive = true);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Images")
	int32 ImportRuntimeImagesAsElementsFromDirectory(const FString& Directory, FVector2D StartPosition, int32 Columns = 4, float ElementSpacing = 220.0f, FVector2D ElementSize = FVector2D(180.0f, 180.0f));

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	bool SetElementText(const FString& ElementId, const FString& Text);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	bool SetElementFillColor(const FString& ElementId, FLinearColor FillColor);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	bool SetElementBorderColor(const FString& ElementId, FLinearColor BorderColor);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	bool SetElementTextStyle(const FString& ElementId, FLinearColor TextColor, float FontSize);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	bool SetElementTransform(const FString& ElementId, FVector2D Position, FVector2D Size, float Rotation);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	TArray<FString> GetSelectedElementIds() const { return SelectedElementIds; }

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Elements")
	int32 DeleteSelectedElements();

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Selection")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Connections")
	void SetConnectionRoutingMode(EGraphDesignerConnectionRoutingMode InRoutingMode);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Connections")
	EGraphDesignerConnectionRoutingMode GetConnectionRoutingMode() const { return ConnectionRoutingMode; }

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Connections")
	bool SetConnectionLineStyle(const FString& ConnectionId, EGraphDesignerConnectionLineStyle LineStyle);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	void SetRequiredConnections(const TArray<FGraphDesignerRequiredConnection>& InRequiredConnections);

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	TArray<FGraphDesignerRequiredConnection> GetRequiredConnections() const;

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Answers")
	bool IsRequiredConnection(const FString& StartPinId, const FString& EndPinId) const;

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

	UFUNCTION(BlueprintCallable, Category = "GraphDesigner|Serialization")
	bool LoadGraph(const FString& GraphName);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY(Transient)
	UGraphDocument* Document = nullptr;

	UPROPERTY(Transient)
	TArray<FString> SelectedElementIds;

	TSharedPtr<SGraphDesignerCanvas> GraphCanvas;
	TSharedPtr<SGraphDesignerPalette> PaletteWidget;
	TSharedPtr<SGraphDesignerInspector> InspectorWidget;
	TSharedPtr<SGraphDesignerAnswerPanel> AnswerPanelWidget;
	TSharedPtr<SEditableTextBox> SaveNameTextBox;
	TSharedPtr<SEditableTextBox> LoadNameTextBox;
	bool bSaveDialogVisible = false;
	bool bLoadDialogVisible = false;
	FString RuntimeImageDirectory = FPaths::Combine(FPaths::ProjectContentDir(),TEXT("HuaYang"), TEXT("GraphsImage"));

	void EnsureDocument();
	void InvalidateCanvas();
	FReply OpenSaveDialog();
	FReply ConfirmSaveDialog();
	FReply CancelSaveDialog();
	EVisibility GetSaveDialogVisibility() const;
	FReply OpenLoadDialog();
	FReply ConfirmLoadDialog();
	FReply CancelLoadDialog();
	EVisibility GetLoadDialogVisibility() const;
	void HandleSelectionChanged(const TArray<FString>& InSelectedElementIds);
	void HandleInspectorTextCommitted(const FString& ElementId, const FString& Text);
	void HandleInspectorFillColorCommitted(const FString& ElementId, const FLinearColor& FillColor);
	void HandleInspectorBorderColorCommitted(const FString& ElementId, const FLinearColor& BorderColor);
	void HandleInspectorTextColorCommitted(const FString& ElementId, const FLinearColor& TextColor);
	void HandleInspectorFontSizeCommitted(const FString& ElementId, float FontSize);
	void HandleInspectorRotationCommitted(const FString& ElementId, float Rotation);
	void HandleInspectorPinChanged(const FGraphDesignerPin& Pin);
	void HandleInspectorConnectionChanged(const FGraphDesignerConnection& Connection);
	void HandleInspectorAddPin(const FString& ElementId, FVector2D RelativePosition);
	void HandleInspectorRemoveLastPin(const FString& ElementId);
	void HandleInspectorDeleteSelected();
	void HandlePinSelected(const FString& PinId);
	void HandleConnectionSelected(const FString& ConnectionId);
	void HandleAnswerChanged(int32 Index, const FGraphDesignerRequiredConnection& RequiredConnection);
	void HandleAnswerAdded();
	void HandleAnswerRemoved(int32 Index);
	FString ResolvePinName(const FString& PinId) const;
};
