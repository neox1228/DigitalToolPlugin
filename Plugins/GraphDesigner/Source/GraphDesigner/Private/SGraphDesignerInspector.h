#pragma once

#include "CoreMinimal.h"
#include "GraphDocument.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;

//设置面板文本提交广播代理
DECLARE_DELEGATE_TwoParams(FOnGraphDesignerInspectorTextCommitted, const FString& /*ElementId*/, const FString& /*Text*/)
DECLARE_DELEGATE_TwoParams(FOnGraphDesignerInspectorFillColorCommitted, const FString& /*ElementId*/, const FLinearColor& /*FillColor*/)
DECLARE_DELEGATE_TwoParams(FOnGraphDesignerInspectorBorderColorCommitted, const FString& /*ElementId*/, const FLinearColor& /*BorderColor*/)
DECLARE_DELEGATE_TwoParams(FOnGraphDesignerInspectorTextColorCommitted, const FString& /*ElementId*/, const FLinearColor& /*TextColor*/)
DECLARE_DELEGATE_TwoParams(FOnGraphDesignerInspectorFontSizeCommitted, const FString& /*ElementId*/, float /*FontSize*/)
DECLARE_DELEGATE_TwoParams(FOnGraphDesignerInspectorRotationCommitted, const FString& /*ElementId*/, float /*Rotation*/)
DECLARE_DELEGATE_OneParam(FOnGraphDesignerInspectorPinChanged, const FGraphDesignerPin& /*Pin*/)
DECLARE_DELEGATE_OneParam(FOnGraphDesignerInspectorConnectionChanged, const FGraphDesignerConnection& /*Connection*/)
DECLARE_DELEGATE_TwoParams(FOnGraphDesignerInspectorAddPin, const FString& /*ElementId*/, FVector2D /*RelativePosition*/)
DECLARE_DELEGATE_OneParam(FOnGraphDesignerInspectorRemoveLastPin, const FString& /*ElementId*/)
DECLARE_DELEGATE(FOnGraphDesignerInspectorDeleteSelected)

class SGraphDesignerInspector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGraphDesignerInspector)
		: _Document(nullptr)
	{
	}
		SLATE_ARGUMENT(UGraphDocument*, Document)
		SLATE_EVENT(FOnGraphDesignerInspectorTextCommitted, OnTextCommitted)
		SLATE_EVENT(FOnGraphDesignerInspectorFillColorCommitted, OnFillColorCommitted)
		SLATE_EVENT(FOnGraphDesignerInspectorBorderColorCommitted, OnBorderColorCommitted)
		SLATE_EVENT(FOnGraphDesignerInspectorTextColorCommitted, OnTextColorCommitted)
		SLATE_EVENT(FOnGraphDesignerInspectorFontSizeCommitted, OnFontSizeCommitted)
		SLATE_EVENT(FOnGraphDesignerInspectorRotationCommitted, OnRotationCommitted)
		SLATE_EVENT(FOnGraphDesignerInspectorPinChanged, OnPinChanged)
		SLATE_EVENT(FOnGraphDesignerInspectorConnectionChanged, OnConnectionChanged)
		SLATE_EVENT(FOnGraphDesignerInspectorAddPin, OnAddPin)
		SLATE_EVENT(FOnGraphDesignerInspectorRemoveLastPin, OnRemoveLastPin)
		SLATE_EVENT(FOnGraphDesignerInspectorDeleteSelected, OnDeleteSelected)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetDocument(UGraphDocument* InDocument);
	void SetSelectedElementId(const FString& InElementId);
	void SetSelectedPinId(const FString& InPinId);
	void SetSelectedConnectionId(const FString& InConnectionId);

private:
	enum class EPinSide
	{
		Left,
		Right,
		Top,
		Bottom
	};

	enum class EColorTarget
	{
		Fill,
		Border,
		Text,
		PinFill,
		PinBorder,
		PinText,
		Connection
	};

	void RebuildInspector();
	void RebuildElementInspector(FGraphDesignerElement& Element);
	void RebuildPinInspector(FGraphDesignerPin& Pin);
	void RebuildConnectionInspector(FGraphDesignerConnection& Connection);
	void AddColorEditor(const TCHAR* Label, const FLinearColor& Color, EColorTarget Target);
	FReply AddPin(EPinSide Side);
	FReply RemoveLastPin();
	FReply DeleteSelected();
	void CommitText(const FText& NewText, ETextCommit::Type CommitType);
	void CommitPinText(const FText& NewText, ETextCommit::Type CommitType);
	void CommitPinFontSize(const FText& NewText, ETextCommit::Type CommitType);
	void CommitConnectionThickness(const FText& NewText, ETextCommit::Type CommitType);
	FReply SetConnectionLineStyle(EGraphDesignerConnectionLineStyle LineStyle);
	FReply SetPinTextPosition(EGraphDesignerPinTextPosition TextPosition);
	void CommitColorComponent(float NewValue, ETextCommit::Type CommitType, EColorTarget Target, int32 ComponentIndex);
	void ChangeColorComponent(float NewValue, EColorTarget Target, int32 ComponentIndex);
	void CommitFontSize(const FText& NewText, ETextCommit::Type CommitType);
	void CommitRotation(float NewValue, ETextCommit::Type CommitType);
	FVector2D GetNextPinPosition(const FGraphDesignerElement& Element, EPinSide Side) const;

	TWeakObjectPtr<UGraphDocument> Document;
	FString SelectedElementId;
	FString SelectedPinId;
	FString SelectedConnectionId;
	TSharedPtr<SVerticalBox> ContentBox;
	FOnGraphDesignerInspectorTextCommitted OnTextCommitted;
	FOnGraphDesignerInspectorFillColorCommitted OnFillColorCommitted;
	FOnGraphDesignerInspectorBorderColorCommitted OnBorderColorCommitted;
	FOnGraphDesignerInspectorTextColorCommitted OnTextColorCommitted;
	FOnGraphDesignerInspectorFontSizeCommitted OnFontSizeCommitted;
	FOnGraphDesignerInspectorRotationCommitted OnRotationCommitted;
	FOnGraphDesignerInspectorPinChanged OnPinChanged;
	FOnGraphDesignerInspectorConnectionChanged OnConnectionChanged;
	FOnGraphDesignerInspectorAddPin OnAddPin;
	FOnGraphDesignerInspectorRemoveLastPin OnRemoveLastPin;
	FOnGraphDesignerInspectorDeleteSelected OnDeleteSelected;
};
