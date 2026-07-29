#pragma once

#include "CoreMinimal.h"
#include "GraphDocument.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;

DECLARE_DELEGATE_TwoParams(FOnGraphDesignerAnswerChanged, int32 /*Index*/, const FGraphDesignerRequiredConnection& /*RequiredConnection*/)
DECLARE_DELEGATE(FOnGraphDesignerAnswerAdded)
DECLARE_DELEGATE_OneParam(FOnGraphDesignerAnswerRemoved, int32 /*Index*/)

class SGraphDesignerAnswerPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGraphDesignerAnswerPanel)
		: _Document(nullptr)
	{
	}
		SLATE_ARGUMENT(UGraphDocument*, Document)
		SLATE_EVENT(FOnGraphDesignerAnswerChanged, OnAnswerChanged)
		SLATE_EVENT(FOnGraphDesignerAnswerAdded, OnAnswerAdded)
		SLATE_EVENT(FOnGraphDesignerAnswerRemoved, OnAnswerRemoved)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetDocument(UGraphDocument* InDocument);
	void RebuildPanel();

private:
	void CommitStartPinId(const FText& NewText, ETextCommit::Type CommitType, int32 Index);
	void CommitEndPinId(const FText& NewText, ETextCommit::Type CommitType, int32 Index);
	FReply AddAnswer();
	FReply RemoveAnswer(int32 Index);

	TWeakObjectPtr<UGraphDocument> Document;
	TSharedPtr<SVerticalBox> ContentBox;
	FOnGraphDesignerAnswerChanged OnAnswerChanged;
	FOnGraphDesignerAnswerAdded OnAnswerAdded;
	FOnGraphDesignerAnswerRemoved OnAnswerRemoved;
};
