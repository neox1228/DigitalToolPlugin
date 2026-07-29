#include "SGraphDesignerAnswerPanel.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SGraphDesignerAnswerPanel::Construct(const FArguments& InArgs)
{
	Document = InArgs._Document;
	OnAnswerChanged = InArgs._OnAnswerChanged;
	OnAnswerAdded = InArgs._OnAnswerAdded;
	OnAnswerRemoved = InArgs._OnAnswerRemoved;

	ChildSlot
	[
		SNew(SBorder)
		.Padding(8.0f)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(ContentBox, SVerticalBox)
			]
		]
	];

	RebuildPanel();
}

void SGraphDesignerAnswerPanel::SetDocument(UGraphDocument* InDocument)
{
	Document = InDocument;
	RebuildPanel();
}

void SGraphDesignerAnswerPanel::RebuildPanel()
{
	if (!ContentBox.IsValid())
	{
		return;
	}

	ContentBox->ClearChildren();

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("答案数组")))
	];

	UGraphDocument* GraphDocument = Document.Get();
	if (GraphDocument == nullptr)
	{
		ContentBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("No document.")))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.68f, 0.72f, 1.0f)))
		];
		return;
	}

	for (int32 Index = 0; Index < GraphDocument->RequiredConnections.Num(); ++Index)
	{
		const FGraphDesignerRequiredConnection& RequiredConnection = GraphDocument->RequiredConnections[Index];

		ContentBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("(%d) startid / endid"), Index)))
		];

		ContentBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(RequiredConnection.StartPinId))
				.HintText(FText::FromString(TEXT("startid")))
				.OnTextCommitted(this, &SGraphDesignerAnswerPanel::CommitStartPinId, Index)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(RequiredConnection.EndPinId))
				.HintText(FText::FromString(TEXT("endid")))
				.OnTextCommitted(this, &SGraphDesignerAnswerPanel::CommitEndPinId, Index)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("X")))
				.OnClicked(this, &SGraphDesignerAnswerPanel::RemoveAnswer, Index)
			]
		];
	}

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 4.0f, 0.0f, 0.0f)
	[
		SNew(SButton)
		.Text(FText::FromString(TEXT("+ 添加答案")))
		.OnClicked(this, &SGraphDesignerAnswerPanel::AddAnswer)
	];
}

void SGraphDesignerAnswerPanel::CommitStartPinId(const FText& NewText, ETextCommit::Type CommitType, int32 Index)
{
	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (GraphDocument->RequiredConnections.IsValidIndex(Index))
		{
			FGraphDesignerRequiredConnection RequiredConnection = GraphDocument->RequiredConnections[Index];
			RequiredConnection.StartPinId = NewText.ToString().TrimStartAndEnd();
			OnAnswerChanged.ExecuteIfBound(Index, RequiredConnection);
		}
	}
}

void SGraphDesignerAnswerPanel::CommitEndPinId(const FText& NewText, ETextCommit::Type CommitType, int32 Index)
{
	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (GraphDocument->RequiredConnections.IsValidIndex(Index))
		{
			FGraphDesignerRequiredConnection RequiredConnection = GraphDocument->RequiredConnections[Index];
			RequiredConnection.EndPinId = NewText.ToString().TrimStartAndEnd();
			OnAnswerChanged.ExecuteIfBound(Index, RequiredConnection);
		}
	}
}

FReply SGraphDesignerAnswerPanel::AddAnswer()
{
	OnAnswerAdded.ExecuteIfBound();
	RebuildPanel();
	return FReply::Handled();
}

FReply SGraphDesignerAnswerPanel::RemoveAnswer(int32 Index)
{
	OnAnswerRemoved.ExecuteIfBound(Index);
	RebuildPanel();
	return FReply::Handled();
}
