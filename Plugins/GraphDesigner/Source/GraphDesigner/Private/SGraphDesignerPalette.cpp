#include "SGraphDesignerPalette.h"

#include "GraphDesignerDragDropOp.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace GraphDesignerPalette
{
	constexpr float ItemPadding = 8.0f;
}

class SGraphDesignerPaletteItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGraphDesignerPaletteItem)
	{
	}
		SLATE_ARGUMENT(FGraphDesignerNodeTemplate, NodeTemplate)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		NodeTemplate = InArgs._NodeTemplate;

		ChildSlot
		[
			SNew(SBorder)
			.Padding(GraphDesignerPalette::ItemPadding)
			.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(NodeTemplate.DisplayName))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(GetTypeText(NodeTemplate.Type))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.68f, 0.72f, 1.0f)))
				]
			]
		];
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			return FReply::Handled().DetectDrag(AsShared(), EKeys::LeftMouseButton);
		}

		return FReply::Unhandled();
	}

	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		return FReply::Handled().BeginDragDrop(FGraphDesignerTemplateDragDropOp::New(NodeTemplate));
	}

private:
	static FText GetTypeText(EGraphDesignerElementType Type)
	{
		switch (Type)
		{
		case EGraphDesignerElementType::Rectangle:
			return FText::FromString(TEXT("矩形"));
		case EGraphDesignerElementType::RoundedRectangle:
			return FText::FromString(TEXT("圆角矩形"));
		case EGraphDesignerElementType::Circle:
			return FText::FromString(TEXT("圆形"));
		case EGraphDesignerElementType::Ellipse:
			return FText::FromString(TEXT("椭圆"));
		case EGraphDesignerElementType::Triangle:
			return FText::FromString(TEXT("三角形"));
		case EGraphDesignerElementType::RightTriangle:
			return FText::FromString(TEXT("直角三角形"));
		case EGraphDesignerElementType::Diamond:
			return FText::FromString(TEXT("菱形"));
		case EGraphDesignerElementType::Parallelogram:
			return FText::FromString(TEXT("平行四边形"));
		case EGraphDesignerElementType::Trapezoid:
			return FText::FromString(TEXT("梯形"));
		case EGraphDesignerElementType::Pentagon:
			return FText::FromString(TEXT("五边形"));
		case EGraphDesignerElementType::Hexagon:
			return FText::FromString(TEXT("六边形"));
		case EGraphDesignerElementType::Octagon:
			return FText::FromString(TEXT("八边形"));
		case EGraphDesignerElementType::Star:
			return FText::FromString(TEXT("星形"));
		case EGraphDesignerElementType::Text:
			return FText::FromString(TEXT("文本"));
		case EGraphDesignerElementType::Image:
			return FText::FromString(TEXT("图片"));
		case EGraphDesignerElementType::Line:
			return FText::FromString(TEXT("线段"));
		case EGraphDesignerElementType::Arrow:
			return FText::FromString(TEXT("箭头"));
		case EGraphDesignerElementType::Pin:
			return FText::FromString(TEXT("连接点"));
		default:
			return FText::FromString(TEXT("图元"));
		}
	}

public:
	static FString GetCategoryText(const FGraphDesignerNodeTemplate& NodeTemplate)
	{
		const FString TemplateId = NodeTemplate.TemplateId.ToString();
		if (TemplateId == TEXT("Server") || TemplateId == TEXT("Switch") || TemplateId == TEXT("Router"))
		{
			return TEXT("设备图元");
		}

		switch (NodeTemplate.Type)
		{
		case EGraphDesignerElementType::Line:
		case EGraphDesignerElementType::Arrow:
			return TEXT("连线图元");
		case EGraphDesignerElementType::Text:
		case EGraphDesignerElementType::Image:
		case EGraphDesignerElementType::Pin:
			return TEXT("标注与连接");
		default:
			return TEXT("基础图形");
		}
	}

private:
	FGraphDesignerNodeTemplate NodeTemplate;
};

void SGraphDesignerPalette::Construct(const FArguments& InArgs)
{
	NodeTemplates = InArgs._NodeTemplates;

	ChildSlot
	[
		SNew(SBorder)
		.Padding(8.0f)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(ItemBox, SVerticalBox)
			]
		]
	];

	RebuildPalette();
}

void SGraphDesignerPalette::SetNodeTemplates(const TArray<FGraphDesignerNodeTemplate>& InNodeTemplates)
{
	NodeTemplates = InNodeTemplates;
	RebuildPalette();
}

void SGraphDesignerPalette::RebuildPalette()
{
	if (!ItemBox.IsValid())
	{
		return;
	}

	ItemBox->ClearChildren();

	ItemBox->AddSlot()
	.AutoHeight()
	.Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(STextBlock)
		.Text(FText::FromString(TEXT("图元面板")))
	];

	FString LastCategory;
	for (const FGraphDesignerNodeTemplate& NodeTemplate : NodeTemplates)
	{
		const FString Category = SGraphDesignerPaletteItem::GetCategoryText(NodeTemplate);
		if (Category != LastCategory)
		{
			ItemBox->AddSlot()
			.AutoHeight()
			.Padding(0.0f, LastCategory.IsEmpty() ? 0.0f : 8.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Category))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.88f, 0.95f, 1.0f)))
			];
			LastCategory = Category;
		}

		ItemBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 6.0f)
		[
			SNew(SGraphDesignerPaletteItem)
			.NodeTemplate(NodeTemplate)
		];
	}
}
