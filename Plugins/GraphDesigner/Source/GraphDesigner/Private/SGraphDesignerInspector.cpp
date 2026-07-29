#include "SGraphDesignerInspector.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SGraphDesignerInspector::Construct(const FArguments& InArgs)
{
	Document = InArgs._Document;
	OnTextCommitted = InArgs._OnTextCommitted;
	OnFillColorCommitted = InArgs._OnFillColorCommitted;
	OnBorderColorCommitted = InArgs._OnBorderColorCommitted;
	OnTextColorCommitted = InArgs._OnTextColorCommitted;
	OnFontSizeCommitted = InArgs._OnFontSizeCommitted;
	OnRotationCommitted = InArgs._OnRotationCommitted;
	OnPinChanged = InArgs._OnPinChanged;
	OnConnectionChanged = InArgs._OnConnectionChanged;
	OnAddPin = InArgs._OnAddPin;
	OnRemoveLastPin = InArgs._OnRemoveLastPin;
	OnDeleteSelected = InArgs._OnDeleteSelected;

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

	RebuildInspector();
}

void SGraphDesignerInspector::SetDocument(UGraphDocument* InDocument)
{
	Document = InDocument;
	SelectedElementId.Reset();
	SelectedPinId.Reset();
	SelectedConnectionId.Reset();
	RebuildInspector();
}

void SGraphDesignerInspector::SetSelectedElementId(const FString& InElementId)
{
	SelectedElementId = InElementId;
	SelectedPinId.Reset();
	SelectedConnectionId.Reset();
	RebuildInspector();
}

void SGraphDesignerInspector::SetSelectedPinId(const FString& InPinId)
{
	SelectedPinId = InPinId;
	SelectedElementId.Reset();
	SelectedConnectionId.Reset();
	RebuildInspector();
}

void SGraphDesignerInspector::SetSelectedConnectionId(const FString& InConnectionId)
{
	SelectedConnectionId = InConnectionId;
	SelectedElementId.Reset();
	SelectedPinId.Reset();
	RebuildInspector();
}

void SGraphDesignerInspector::RebuildInspector()
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
		.Text(FText::FromString(TEXT("Inspector")))
	];

	UGraphDocument* GraphDocument = Document.Get();
	FGraphDesignerPin* Pin = GraphDocument != nullptr ? GraphDocument->FindPin(SelectedPinId) : nullptr;
	if (Pin != nullptr)
	{
		RebuildPinInspector(*Pin);
		return;
	}

	FGraphDesignerConnection* Connection = GraphDocument != nullptr ? GraphDocument->FindConnection(SelectedConnectionId) : nullptr;
	if (Connection != nullptr)
	{
		RebuildConnectionInspector(*Connection);
		return;
	}

	FGraphDesignerElement* Element = GraphDocument != nullptr ? GraphDocument->FindElement(SelectedElementId) : nullptr;
	if (Element == nullptr)
	{
		ContentBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Select a node to edit it.")))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.68f, 0.72f, 1.0f)))
		];
		return;
	}

	RebuildElementInspector(*Element);
}

void SGraphDesignerInspector::RebuildConnectionInspector(FGraphDesignerConnection& Connection)
{
	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Connection")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(FString::Printf(TEXT("Start: %s"), *Connection.StartPinId)))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.68f, 0.72f, 1.0f)))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(FString::Printf(TEXT("End: %s"), *Connection.EndPinId)))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.68f, 0.72f, 1.0f)))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Line Style")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(Connection.LineStyle == EGraphDesignerConnectionLineStyle::Solid ? TEXT("Solid *") : TEXT("Solid")))
			.OnClicked(this, &SGraphDesignerInspector::SetConnectionLineStyle, EGraphDesignerConnectionLineStyle::Solid)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(Connection.LineStyle == EGraphDesignerConnectionLineStyle::Dashed ? TEXT("Dashed *") : TEXT("Dashed")))
			.OnClicked(this, &SGraphDesignerInspector::SetConnectionLineStyle, EGraphDesignerConnectionLineStyle::Dashed)
		]
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Thickness")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(SEditableTextBox)
		.Text(FText::AsNumber(Connection.Thickness))
		.OnTextCommitted(this, &SGraphDesignerInspector::CommitConnectionThickness)
	];

	AddColorEditor(TEXT("Connection Color"), Connection.Color, EColorTarget::Connection);
}

void SGraphDesignerInspector::RebuildElementInspector(FGraphDesignerElement& Element)
{
	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Text")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(SEditableTextBox)
		.Text(FText::FromString(Element.Text))
		.OnTextCommitted(this, &SGraphDesignerInspector::CommitText)
	];

	AddColorEditor(TEXT("Fill Color"), Element.FillColor, EColorTarget::Fill);
	AddColorEditor(TEXT("Border Color"), Element.BorderColor, EColorTarget::Border);
	AddColorEditor(TEXT("Text Color"), Element.TextColor, EColorTarget::Text);

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Font Size")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(SEditableTextBox)
		.Text(FText::AsNumber(Element.FontSize))
		.OnTextCommitted(this, &SGraphDesignerInspector::CommitFontSize)
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Rotation")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(SNumericEntryBox<float>)
		.Value(TOptional<float>(Element.Rotation))
		.Delta(1.0f)
		.MinSliderValue(-360.0f)
		.MaxSliderValue(360.0f)
		.OnValueCommitted(this, &SGraphDesignerInspector::CommitRotation)
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(FString::Printf(TEXT("Pins: %d"), Element.Pins.Num())))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("+ Left")))
			.OnClicked(this, &SGraphDesignerInspector::AddPin, EPinSide::Left)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("+ Right")))
			.OnClicked(this, &SGraphDesignerInspector::AddPin, EPinSide::Right)
		]
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("+ Top")))
			.OnClicked(this, &SGraphDesignerInspector::AddPin, EPinSide::Top)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("+ Bottom")))
			.OnClicked(this, &SGraphDesignerInspector::AddPin, EPinSide::Bottom)
		]
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 8.0f, 0.0f, 0.0f)
	[
		SNew(SButton)
		.Text(FText::FromString(TEXT("Remove Last Pin")))
		.OnClicked(this, &SGraphDesignerInspector::RemoveLastPin)
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 12.0f, 0.0f, 0.0f)
	[
		SNew(SButton)
		.Text(FText::FromString(TEXT("Delete Selected Element")))
		.OnClicked(this, &SGraphDesignerInspector::DeleteSelected)
	];
}

void SGraphDesignerInspector::RebuildPinInspector(FGraphDesignerPin& Pin)
{
	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Pin Text")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(SEditableTextBox)
		.Text(FText::FromString(Pin.Text))
		.OnTextCommitted(this, &SGraphDesignerInspector::CommitPinText)
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Pin Size")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SEditableTextBox)
			.Text(FText::AsNumber(Pin.Size.X))
			.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
			{
				if (UGraphDocument* GraphDocument = Document.Get())
				{
					if (FGraphDesignerPin* CurrentPin = GraphDocument->FindPin(SelectedPinId))
					{
						FGraphDesignerPin UpdatedPin = *CurrentPin;
						UpdatedPin.Size.X = FMath::Max(1.0f, FCString::Atof(*NewText.ToString()));
						OnPinChanged.ExecuteIfBound(UpdatedPin);
					}
				}
			})
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SEditableTextBox)
			.Text(FText::AsNumber(Pin.Size.Y))
			.OnTextCommitted_Lambda([this](const FText& NewText, ETextCommit::Type)
			{
				if (UGraphDocument* GraphDocument = Document.Get())
				{
					if (FGraphDesignerPin* CurrentPin = GraphDocument->FindPin(SelectedPinId))
					{
						FGraphDesignerPin UpdatedPin = *CurrentPin;
						UpdatedPin.Size.Y = FMath::Max(1.0f, FCString::Atof(*NewText.ToString()));
						OnPinChanged.ExecuteIfBound(UpdatedPin);
					}
				}
			})
		]
	];

	AddColorEditor(TEXT("Pin Fill Color"), Pin.FillColor, EColorTarget::PinFill);
	AddColorEditor(TEXT("Pin Border Color"), Pin.BorderColor, EColorTarget::PinBorder);
	AddColorEditor(TEXT("Pin Text Color"), Pin.TextColor, EColorTarget::PinText);

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Pin Font Size")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 12.0f)
	[
		SNew(SEditableTextBox)
		.Text(FText::AsNumber(Pin.FontSize))
		.OnTextCommitted(this, &SGraphDesignerInspector::CommitPinFontSize)
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("Pin Text Position")))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Left"))).OnClicked(this, &SGraphDesignerInspector::SetPinTextPosition, EGraphDesignerPinTextPosition::Left)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Right"))).OnClicked(this, &SGraphDesignerInspector::SetPinTextPosition, EGraphDesignerPinTextPosition::Right)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Top"))).OnClicked(this, &SGraphDesignerInspector::SetPinTextPosition, EGraphDesignerPinTextPosition::Top)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SButton).Text(FText::FromString(TEXT("Bottom"))).OnClicked(this, &SGraphDesignerInspector::SetPinTextPosition, EGraphDesignerPinTextPosition::Bottom)
		]
	];
}

void SGraphDesignerInspector::AddColorEditor(const TCHAR* Label, const FLinearColor& Color, EColorTarget Target)
{
	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(Label))
	];

	ContentBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(SNumericEntryBox<float>)
			.LabelVAlign(VAlign_Center)
			.Label()
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("R")))
			]
			.Value(Color.R)
			.MinValue(0.0f)
			.MaxValue(1.0f)
			.MinSliderValue(0.0f)
			.MaxSliderValue(1.0f)
			.Delta(0.01f)
			.OnValueChanged(this, &SGraphDesignerInspector::ChangeColorComponent, Target, 0)
			.OnValueCommitted(this, &SGraphDesignerInspector::CommitColorComponent, Target, 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(SNumericEntryBox<float>)
			.LabelVAlign(VAlign_Center)
			.Label()
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("G")))
			]
			.Value(Color.G)
			.MinValue(0.0f)
			.MaxValue(1.0f)
			.MinSliderValue(0.0f)
			.MaxSliderValue(1.0f)
			.Delta(0.01f)
			.OnValueChanged(this, &SGraphDesignerInspector::ChangeColorComponent, Target, 1)
			.OnValueCommitted(this, &SGraphDesignerInspector::CommitColorComponent, Target, 1)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 0.0f, 3.0f, 0.0f)
		[
			SNew(SNumericEntryBox<float>)
			.LabelVAlign(VAlign_Center)
			.Label()
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("B")))
			]
			.Value(Color.B)
			.MinValue(0.0f)
			.MaxValue(1.0f)
			.MinSliderValue(0.0f)
			.MaxSliderValue(1.0f)
			.Delta(0.01f)
			.OnValueChanged(this, &SGraphDesignerInspector::ChangeColorComponent, Target, 2)
			.OnValueCommitted(this, &SGraphDesignerInspector::CommitColorComponent, Target, 2)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SNumericEntryBox<float>)
			.LabelVAlign(VAlign_Center)
			.Label()
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("A")))
			]
			.Value(Color.A)
			.MinValue(0.0f)
			.MaxValue(1.0f)
			.MinSliderValue(0.0f)
			.MaxSliderValue(1.0f)
			.Delta(0.01f)
			.OnValueChanged(this, &SGraphDesignerInspector::ChangeColorComponent, Target, 3)
			.OnValueCommitted(this, &SGraphDesignerInspector::CommitColorComponent, Target, 3)
		]
	];
}

FReply SGraphDesignerInspector::AddPin(EPinSide Side)
{
	UGraphDocument* GraphDocument = Document.Get();
	FGraphDesignerElement* Element = GraphDocument != nullptr ? GraphDocument->FindElement(SelectedElementId) : nullptr;
	if (Element != nullptr)
	{
		OnAddPin.ExecuteIfBound(SelectedElementId, GetNextPinPosition(*Element, Side));
		RebuildInspector();
	}

	return FReply::Handled();
}

FReply SGraphDesignerInspector::RemoveLastPin()
{
	OnRemoveLastPin.ExecuteIfBound(SelectedElementId);
	RebuildInspector();
	return FReply::Handled();
}

FReply SGraphDesignerInspector::DeleteSelected()
{
	OnDeleteSelected.ExecuteIfBound();
	return FReply::Handled();
}

void SGraphDesignerInspector::CommitText(const FText& NewText, ETextCommit::Type CommitType)
{
	OnTextCommitted.ExecuteIfBound(SelectedElementId, NewText.ToString());
}

void SGraphDesignerInspector::CommitPinText(const FText& NewText, ETextCommit::Type CommitType)
{
	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (FGraphDesignerPin* Pin = GraphDocument->FindPin(SelectedPinId))
		{
			FGraphDesignerPin UpdatedPin = *Pin;
			UpdatedPin.Text = NewText.ToString();
			OnPinChanged.ExecuteIfBound(UpdatedPin);
			if (FGraphDesignerElement* OwnerElement = GraphDocument->FindElement(UpdatedPin.OwnerElementId))
			{
				for (const FGraphDesignerPin& OwnerPin : OwnerElement->Pins)
				{
					if (OwnerPin.Text == UpdatedPin.Text && OwnerPin.RelativePosition.Equals(UpdatedPin.RelativePosition))
					{
						SelectedPinId = OwnerPin.Id;
						break;
					}
				}
			}
			RebuildInspector();
		}
	}
}

void SGraphDesignerInspector::CommitPinFontSize(const FText& NewText, ETextCommit::Type CommitType)
{
	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (FGraphDesignerPin* Pin = GraphDocument->FindPin(SelectedPinId))
		{
			FGraphDesignerPin UpdatedPin = *Pin;
			UpdatedPin.FontSize = FMath::Max(1.0f, FCString::Atof(*NewText.ToString()));
			OnPinChanged.ExecuteIfBound(UpdatedPin);
		}
	}
}

void SGraphDesignerInspector::CommitConnectionThickness(const FText& NewText, ETextCommit::Type CommitType)
{
	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (FGraphDesignerConnection* Connection = GraphDocument->FindConnection(SelectedConnectionId))
		{
			FGraphDesignerConnection UpdatedConnection = *Connection;
			UpdatedConnection.Thickness = FMath::Max(1.0f, FCString::Atof(*NewText.ToString()));
			OnConnectionChanged.ExecuteIfBound(UpdatedConnection);
		}
	}
}

FReply SGraphDesignerInspector::SetConnectionLineStyle(EGraphDesignerConnectionLineStyle LineStyle)
{
	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (FGraphDesignerConnection* Connection = GraphDocument->FindConnection(SelectedConnectionId))
		{
			FGraphDesignerConnection UpdatedConnection = *Connection;
			UpdatedConnection.LineStyle = LineStyle;
			OnConnectionChanged.ExecuteIfBound(UpdatedConnection);
			RebuildInspector();
		}
	}

	return FReply::Handled();
}

FReply SGraphDesignerInspector::SetPinTextPosition(EGraphDesignerPinTextPosition TextPosition)
{
	if (UGraphDocument* GraphDocument = Document.Get())
	{
		if (FGraphDesignerPin* Pin = GraphDocument->FindPin(SelectedPinId))
		{
			FGraphDesignerPin UpdatedPin = *Pin;
			UpdatedPin.TextPosition = TextPosition;
			OnPinChanged.ExecuteIfBound(UpdatedPin);
			RebuildInspector();
		}
	}

	return FReply::Handled();
}

void SGraphDesignerInspector::CommitColorComponent(float NewValue, ETextCommit::Type CommitType, EColorTarget Target, int32 ComponentIndex)
{
	ChangeColorComponent(NewValue, Target, ComponentIndex);
}

void SGraphDesignerInspector::ChangeColorComponent(float NewValue, EColorTarget Target, int32 ComponentIndex)
{
	UGraphDocument* GraphDocument = Document.Get();
	FGraphDesignerElement* Element = GraphDocument != nullptr ? GraphDocument->FindElement(SelectedElementId) : nullptr;
	if (Element == nullptr)
	{
		if (FGraphDesignerConnection* Connection = GraphDocument != nullptr ? GraphDocument->FindConnection(SelectedConnectionId) : nullptr)
		{
			FGraphDesignerConnection UpdatedConnection = *Connection;
			switch (ComponentIndex)
			{
			case 0:
				UpdatedConnection.Color.R = NewValue;
				break;
			case 1:
				UpdatedConnection.Color.G = NewValue;
				break;
			case 2:
				UpdatedConnection.Color.B = NewValue;
				break;
			case 3:
				UpdatedConnection.Color.A = NewValue;
				break;
			default:
				break;
			}

			OnConnectionChanged.ExecuteIfBound(UpdatedConnection);
			return;
		}

		if (FGraphDesignerPin* Pin = GraphDocument != nullptr ? GraphDocument->FindPin(SelectedPinId) : nullptr)
		{
			FGraphDesignerPin UpdatedPin = *Pin;
			FLinearColor Color = Target == EColorTarget::PinFill ? UpdatedPin.FillColor : Target == EColorTarget::PinBorder ? UpdatedPin.BorderColor : UpdatedPin.TextColor;
			switch (ComponentIndex)
			{
			case 0:
				Color.R = NewValue;
				break;
			case 1:
				Color.G = NewValue;
				break;
			case 2:
				Color.B = NewValue;
				break;
			case 3:
				Color.A = NewValue;
				break;
			default:
				break;
			}

			if (Target == EColorTarget::PinFill)
			{
				UpdatedPin.FillColor = Color;
			}
			else if (Target == EColorTarget::PinBorder)
			{
				UpdatedPin.BorderColor = Color;
			}
			else
			{
				UpdatedPin.TextColor = Color;
			}

			OnPinChanged.ExecuteIfBound(UpdatedPin);
		}
		return;
	}

	FLinearColor Color = Target == EColorTarget::Fill ? Element->FillColor : Target == EColorTarget::Border ? Element->BorderColor : Element->TextColor;
	switch (ComponentIndex)
	{
	case 0:
		Color.R = NewValue;
		break;
	case 1:
		Color.G = NewValue;
		break;
	case 2:
		Color.B = NewValue;
		break;
	case 3:
		Color.A = NewValue;
		break;
	default:
		break;
	}

	if (Target == EColorTarget::Fill)
	{
		OnFillColorCommitted.ExecuteIfBound(SelectedElementId, Color);
	}
	else if (Target == EColorTarget::Border)
	{
		OnBorderColorCommitted.ExecuteIfBound(SelectedElementId, Color);
	}
	else
	{
		OnTextColorCommitted.ExecuteIfBound(SelectedElementId, Color);
	}
}

void SGraphDesignerInspector::CommitFontSize(const FText& NewText, ETextCommit::Type CommitType)
{
	const float FontSize = FMath::Max(1.0f, FCString::Atof(*NewText.ToString()));
	OnFontSizeCommitted.ExecuteIfBound(SelectedElementId, FontSize);
}

void SGraphDesignerInspector::CommitRotation(float NewValue, ETextCommit::Type CommitType)
{
	OnRotationCommitted.ExecuteIfBound(SelectedElementId, NewValue);
}

FVector2D SGraphDesignerInspector::GetNextPinPosition(const FGraphDesignerElement& Element, EPinSide Side) const
{
	int32 ExistingOnSide = 0;
	for (const FGraphDesignerPin& Pin : Element.Pins)
	{
		const bool bLeft = FMath::IsNearlyZero(Pin.RelativePosition.X);
		const bool bRight = FMath::IsNearlyEqual(Pin.RelativePosition.X, Element.Size.X);
		const bool bTop = FMath::IsNearlyZero(Pin.RelativePosition.Y);
		const bool bBottom = FMath::IsNearlyEqual(Pin.RelativePosition.Y, Element.Size.Y);

		if ((Side == EPinSide::Left && bLeft)
			|| (Side == EPinSide::Right && bRight)
			|| (Side == EPinSide::Top && bTop)
			|| (Side == EPinSide::Bottom && bBottom))
		{
			++ExistingOnSide;
		}
	}

	const float Ratio = static_cast<float>(ExistingOnSide + 1) / static_cast<float>(ExistingOnSide + 2);
	switch (Side)
	{
	case EPinSide::Left:
		return FVector2D(0.0f, Element.Size.Y * Ratio);
	case EPinSide::Right:
		return FVector2D(Element.Size.X, Element.Size.Y * Ratio);
	case EPinSide::Top:
		return FVector2D(Element.Size.X * Ratio, 0.0f);
	case EPinSide::Bottom:
		return FVector2D(Element.Size.X * Ratio, Element.Size.Y);
	default:
		return Element.Size * 0.5f;
	}
}
