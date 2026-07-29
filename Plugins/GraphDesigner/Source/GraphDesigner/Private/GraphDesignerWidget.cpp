#include "GraphDesignerWidget.h"

#include "Blueprint/WidgetTree.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "SGraphDesignerAnswerPanel.h"
#include "SGraphDesignerInspector.h"
#include "SGraphDesignerPalette.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace GraphDesignerWidget
{
	FGraphDesignerNodeTemplate MakeNodeTemplate(
		FName TemplateId,
		const FString& DisplayName,
		EGraphDesignerElementType Type,
		FVector2D Size,
		FLinearColor FillColor,
		FLinearColor BorderColor
	)
	{
		FGraphDesignerNodeTemplate NodeTemplate;
		NodeTemplate.TemplateId = TemplateId;
		NodeTemplate.DisplayName = DisplayName;
		NodeTemplate.Type = Type;
		NodeTemplate.DefaultSize = Size;
		NodeTemplate.FillColor = FillColor;
		NodeTemplate.BorderColor = BorderColor;
		NodeTemplate.Text = DisplayName;
		NodeTemplate.TextColor = FLinearColor::Black;
		NodeTemplate.FontSize = 16.0f;
		return NodeTemplate;
	}

	static bool IsRuntimeImageFile(const FString& FilePath)
	{
		const FString Extension = FPaths::GetExtension(FilePath).ToLower();
		return Extension == TEXT("png")
			|| Extension == TEXT("jpg")
			|| Extension == TEXT("jpeg")
			|| Extension == TEXT("bmp")
			|| Extension == TEXT("tga")
			|| Extension == TEXT("exr")
			|| Extension == TEXT("hdr");
	}

	static FGraphDesignerNodeTemplate MakeImageTemplate(const FString& FilePath)
	{
		FGraphDesignerNodeTemplate Template;
		Template.TemplateId = FName(*FString::Printf(TEXT("Image_%s"), *FPaths::GetBaseFilename(FilePath)));
		Template.DisplayName = FPaths::GetBaseFilename(FilePath);
		Template.Type = EGraphDesignerElementType::Image;
		Template.DefaultSize = FVector2D(180.0f, 180.0f);
		Template.FillColor = FLinearColor::White;
		Template.BorderColor = FLinearColor(0.85f, 0.85f, 0.85f, 1.0f);
		Template.Text = Template.DisplayName;
		Template.TextColor = FLinearColor::White;
		Template.FontSize = 14.0f;
		Template.ImagePath = UGraphDocument::MakeRelativeImagePath(FilePath);
		Template.Pins.Reset();
		return Template;
	}
}

UGraphDesignerWidget::UGraphDesignerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	ResetToDefaultTemplates();
}

void UGraphDesignerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureDocument();

	if (NodeTemplates.IsEmpty())
	{
		ResetToDefaultTemplates();
	}

	if (bAutoLoadRuntimeImages && !RuntimeImageDirectory.IsEmpty())
	{
		LoadRuntimeImageTemplatesFromDirectory(RuntimeImageDirectory, true);
	}
}

void UGraphDesignerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	GraphCanvas.Reset();
	PaletteWidget.Reset();
	InspectorWidget.Reset();
	AnswerPanelWidget.Reset();
}

void UGraphDesignerWidget::SetDocument(UGraphDocument* InDocument)
{
	Document = InDocument;
	EnsureDocument();

	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetDocument(Document);
	}
	if (InspectorWidget.IsValid())
	{
		InspectorWidget->SetDocument(Document);
	}
	if (AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->SetDocument(Document);
	}
}

void UGraphDesignerWidget::CreateNewGraph()
{
	Document = NewObject<UGraphDocument>(this);
	SelectedElementIds.Reset();
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetDocument(Document);
	}
	if (InspectorWidget.IsValid())
	{
		InspectorWidget->SetDocument(Document);
	}
	if (AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->SetDocument(Document);
	}
}

void UGraphDesignerWidget::SetNodeTemplates(const TArray<FGraphDesignerNodeTemplate>& InNodeTemplates)
{
	NodeTemplates = InNodeTemplates;
	if (PaletteWidget.IsValid())
	{
		PaletteWidget->SetNodeTemplates(NodeTemplates);
	}
}

void UGraphDesignerWidget::ResetToDefaultTemplates()
{
	using namespace GraphDesignerWidget;

	NodeTemplates.Reset();

	FGraphDesignerNodeTemplate Server = MakeNodeTemplate(
		TEXT("Server"),
		TEXT("服务器"),
		EGraphDesignerElementType::Rectangle,
		FVector2D(180.0f, 90.0f),
		FLinearColor(0.16f, 0.24f, 0.34f, 1.0f),
		FLinearColor(0.45f, 0.72f, 1.0f, 1.0f)
	);
	NodeTemplates.Add(Server);

	FGraphDesignerNodeTemplate Switch = MakeNodeTemplate(
		TEXT("Switch"),
		TEXT("交换机"),
		EGraphDesignerElementType::RoundedRectangle,
		FVector2D(220.0f, 90.0f),
		FLinearColor(0.16f, 0.30f, 0.22f, 1.0f),
		FLinearColor(0.40f, 0.95f, 0.58f, 1.0f)
	);
	NodeTemplates.Add(Switch);

	FGraphDesignerNodeTemplate Router = MakeNodeTemplate(
		TEXT("Router"),
		TEXT("路由器"),
		EGraphDesignerElementType::Circle,
		FVector2D(100.0f, 100.0f),
		FLinearColor(0.30f, 0.20f, 0.13f, 1.0f),
		FLinearColor(1.0f, 0.62f, 0.24f, 1.0f)
	);
	NodeTemplates.Add(Router);

	const FLinearColor BasicShapeFill(0.20f, 0.22f, 0.28f, 1.0f);
	const FLinearColor BasicShapeBorder(0.72f, 0.78f, 0.88f, 1.0f);

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Rectangle"),
		TEXT("矩形"),
		EGraphDesignerElementType::Rectangle,
		FVector2D(160.0f, 90.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("RoundedRectangle"),
		TEXT("圆角矩形"),
		EGraphDesignerElementType::RoundedRectangle,
		FVector2D(160.0f, 90.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Circle"),
		TEXT("圆形"),
		EGraphDesignerElementType::Circle,
		FVector2D(96.0f, 96.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Ellipse"),
		TEXT("椭圆"),
		EGraphDesignerElementType::Ellipse,
		FVector2D(160.0f, 96.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Triangle"),
		TEXT("三角形"),
		EGraphDesignerElementType::Triangle,
		FVector2D(130.0f, 110.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("RightTriangle"),
		TEXT("直角三角形"),
		EGraphDesignerElementType::RightTriangle,
		FVector2D(130.0f, 110.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Diamond"),
		TEXT("菱形"),
		EGraphDesignerElementType::Diamond,
		FVector2D(120.0f, 100.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Parallelogram"),
		TEXT("平行四边形"),
		EGraphDesignerElementType::Parallelogram,
		FVector2D(170.0f, 90.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Trapezoid"),
		TEXT("梯形"),
		EGraphDesignerElementType::Trapezoid,
		FVector2D(160.0f, 90.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Pentagon"),
		TEXT("五边形"),
		EGraphDesignerElementType::Pentagon,
		FVector2D(130.0f, 110.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Hexagon"),
		TEXT("六边形"),
		EGraphDesignerElementType::Hexagon,
		FVector2D(150.0f, 95.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Octagon"),
		TEXT("八边形"),
		EGraphDesignerElementType::Octagon,
		FVector2D(120.0f, 120.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Star"),
		TEXT("星形"),
		EGraphDesignerElementType::Star,
		FVector2D(120.0f, 120.0f),
		BasicShapeFill,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Line"),
		TEXT("线段"),
		EGraphDesignerElementType::Line,
		FVector2D(180.0f, 1.0f),
		FLinearColor::Transparent,
		BasicShapeBorder
	));

	NodeTemplates.Add(MakeNodeTemplate(
		TEXT("Arrow"),
		TEXT("箭头"),
		EGraphDesignerElementType::Arrow,
		FVector2D(180.0f, 1.0f),
		FLinearColor::Transparent,
		BasicShapeBorder
	));

	FGraphDesignerNodeTemplate Text = MakeNodeTemplate(
		TEXT("Text"),
		TEXT("文本"),
		EGraphDesignerElementType::Text,
		FVector2D(160.0f, 48.0f),
		FLinearColor::White,
		FLinearColor::White
	);
	Text.Pins.Reset();
	NodeTemplates.Add(Text);

	FGraphDesignerNodeTemplate Pin = MakeNodeTemplate(
		TEXT("Pin"),
		TEXT("连接点"),
		EGraphDesignerElementType::Pin,
		FVector2D(12.0f, 12.0f),
		FLinearColor(0.05f, 0.05f, 0.05f, 1.0f),
		FLinearColor::White
	);
	Pin.Text.Reset();
	Pin.TextColor = FLinearColor::White;
	Pin.FontSize = 12.0f;
	NodeTemplates.Add(Pin);

	if (PaletteWidget.IsValid())
	{
		PaletteWidget->SetNodeTemplates(NodeTemplates);
	}
}

void UGraphDesignerWidget::LoadRuntimeImageTemplatesFromDirectory(const FString& Directory, bool bRecursive)
{
	TArray<FString> FoundFiles;
	if (bRecursive)
	{
		IFileManager::Get().FindFilesRecursive(FoundFiles, *Directory, TEXT("*.*"), true, false);
	}
	else
	{
		IFileManager::Get().FindFiles(FoundFiles, *Directory, TEXT("*.*"));
		for (FString& FilePath : FoundFiles)
		{
			FilePath = FPaths::Combine(Directory, FilePath);
		}
	}

	TArray<FGraphDesignerNodeTemplate> ImageTemplates;
	ImageTemplates.Reserve(FoundFiles.Num());
	for (const FString& FilePath : FoundFiles)
	{
		if (GraphDesignerWidget::IsRuntimeImageFile(FilePath))
		{
			ImageTemplates.Add(GraphDesignerWidget::MakeImageTemplate(FilePath));
		}
	}

	ImageTemplates.Sort([](const FGraphDesignerNodeTemplate& A, const FGraphDesignerNodeTemplate& B)
	{
		return A.DisplayName < B.DisplayName;
	});

	if (ImageTemplates.IsEmpty())
	{
		return;
	}

	NodeTemplates.RemoveAll([](const FGraphDesignerNodeTemplate& Template)
	{
		return Template.Type == EGraphDesignerElementType::Image;
	});

	for (const FGraphDesignerNodeTemplate& Template : ImageTemplates)
	{
		NodeTemplates.Add(Template);
	}

	if (PaletteWidget.IsValid())
	{
		PaletteWidget->SetNodeTemplates(NodeTemplates);
	}
}

int32 UGraphDesignerWidget::ImportRuntimeImagesAsElementsFromDirectory(const FString& Directory, FVector2D StartPosition, int32 Columns, float ElementSpacing, FVector2D ElementSize)
{
	EnsureDocument();

	TArray<FString> FoundFiles;
	IFileManager::Get().FindFilesRecursive(FoundFiles, *Directory, TEXT("*.*"), true, false);

	TArray<FString> ImageFiles;
	for (const FString& FilePath : FoundFiles)
	{
		if (GraphDesignerWidget::IsRuntimeImageFile(FilePath))
		{
			ImageFiles.Add(FilePath);
		}
	}

	ImageFiles.Sort();

	if (ImageFiles.IsEmpty())
	{
		return 0;
	}

	Columns = FMath::Max(1, Columns);
	int32 ImportedCount = 0;
	for (int32 Index = 0; Index < ImageFiles.Num(); ++Index)
	{
		const int32 Row = Index / Columns;
		const int32 Column = Index % Columns;
		const FVector2D Position = StartPosition + FVector2D(Column * ElementSpacing, Row * ElementSpacing);

		const FString ElementId = Document->AddElement(EGraphDesignerElementType::Image, Position, ElementSize);
		if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
		{
			Element->Text = FPaths::GetBaseFilename(ImageFiles[Index]);
			Element->ImagePath = UGraphDocument::MakeRelativeImagePath(ImageFiles[Index]);
			Element->FillColor = FLinearColor::White;
			Element->BorderColor = FLinearColor(0.85f, 0.85f, 0.85f, 1.0f);
			Element->TextColor = FLinearColor::White;
		}
		++ImportedCount;
	}

	InvalidateCanvas();
	return ImportedCount;
}

bool UGraphDesignerWidget::SetElementText(const FString& ElementId, const FString& Text)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->Text = Text;
		Document->RefreshElementPinIds(ElementId);
		InvalidateCanvas();
		return true;
	}
	return false;
}

bool UGraphDesignerWidget::SetElementFillColor(const FString& ElementId, FLinearColor FillColor)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->FillColor = FillColor;
		InvalidateCanvas();
		return true;
	}
	return false;
}

bool UGraphDesignerWidget::SetElementBorderColor(const FString& ElementId, FLinearColor BorderColor)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->BorderColor = BorderColor;
		InvalidateCanvas();
		return true;
	}
	return false;
}

bool UGraphDesignerWidget::SetElementTextStyle(const FString& ElementId, FLinearColor TextColor, float FontSize)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->TextColor = TextColor;
		Element->FontSize = FMath::Max(1.0f, FontSize);
		InvalidateCanvas();
		return true;
	}
	return false;
}

bool UGraphDesignerWidget::SetElementTransform(const FString& ElementId, FVector2D Position, FVector2D Size, float Rotation)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		Element->Position = Position;
		Element->Size = Size;
		Element->Rotation = Rotation;
		InvalidateCanvas();
		return true;
	}
	return false;
}

int32 UGraphDesignerWidget::DeleteSelectedElements()
{
	EnsureDocument();
	const int32 RemovedCount = Document->RemoveElements(SelectedElementIds);
	if (RemovedCount > 0)
	{
		SelectedElementIds.Reset();
		if (InspectorWidget.IsValid())
		{
			InspectorWidget->SetSelectedElementId(TEXT(""));
		}
		OnSelectionChanged.Broadcast(SelectedElementIds);
		if (AnswerPanelWidget.IsValid())
		{
			AnswerPanelWidget->RebuildPanel();
		}
		InvalidateCanvas();
	}
	return RemovedCount;
}

void UGraphDesignerWidget::ClearSelection()
{
	EnsureDocument();
	Document->ClearSelection();
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->ClearSelection();
	}
	HandleSelectionChanged({});
	InvalidateCanvas();
}

void UGraphDesignerWidget::SetConnectionRoutingMode(EGraphDesignerConnectionRoutingMode InRoutingMode)
{
	ConnectionRoutingMode = InRoutingMode;
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetConnectionRoutingMode(ConnectionRoutingMode);
	}
}

bool UGraphDesignerWidget::SetConnectionLineStyle(const FString& ConnectionId, EGraphDesignerConnectionLineStyle LineStyle)
{
	EnsureDocument();
	const bool bUpdated = Document->SetConnectionLineStyle(ConnectionId, LineStyle);
	if (bUpdated)
	{
		InvalidateCanvas();
	}
	return bUpdated;
}

void UGraphDesignerWidget::SetRequiredConnections(const TArray<FGraphDesignerRequiredConnection>& InRequiredConnections)
{
	EnsureDocument();
	Document->SetRequiredConnections(InRequiredConnections);
	if (AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->RebuildPanel();
	}
}

TArray<FGraphDesignerRequiredConnection> UGraphDesignerWidget::GetRequiredConnections() const
{
	return Document != nullptr ? Document->GetRequiredConnections() : TArray<FGraphDesignerRequiredConnection>();
}

bool UGraphDesignerWidget::IsRequiredConnection(const FString& StartPinId, const FString& EndPinId) const
{
	return Document != nullptr && Document->IsRequiredConnection(StartPinId, EndPinId);
}

void UGraphDesignerWidget::SetZoom(float InZoom)
{
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetZoom(InZoom);
	}
}

float UGraphDesignerWidget::GetZoom() const
{
	return GraphCanvas.IsValid() ? GraphCanvas->GetZoom() : 1.0f;
}

void UGraphDesignerWidget::SetCanvasPan(FVector2D InPan)
{
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->SetCanvasPan(InPan);
	}
}

FVector2D UGraphDesignerWidget::GetCanvasPan() const
{
	return GraphCanvas.IsValid() ? GraphCanvas->GetCanvasPan() : FVector2D::ZeroVector;
}

bool UGraphDesignerWidget::SaveGraph(const FString& GraphName)
{
	EnsureDocument();
	return Document->SaveToFile(GraphName);
}

bool UGraphDesignerWidget::LoadGraph(const FString& GraphName)
{
	EnsureDocument();
	const bool bLoaded = Document->LoadFromFile(GraphName);
	if (bLoaded && AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->RebuildPanel();
	}
	InvalidateCanvas();
	return bLoaded;
}

TSharedRef<SWidget> UGraphDesignerWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	EnsureDocument();
	if (NodeTemplates.IsEmpty())
	{
		ResetToDefaultTemplates();
	}

	PaletteWidget = SNew(SGraphDesignerPalette)
		.NodeTemplates(NodeTemplates);

	InspectorWidget = SNew(SGraphDesignerInspector)
		.Document(Document)
		.OnTextCommitted_UObject(this, &UGraphDesignerWidget::HandleInspectorTextCommitted)
		.OnFillColorCommitted_UObject(this, &UGraphDesignerWidget::HandleInspectorFillColorCommitted)
		.OnBorderColorCommitted_UObject(this, &UGraphDesignerWidget::HandleInspectorBorderColorCommitted)
		.OnTextColorCommitted_UObject(this, &UGraphDesignerWidget::HandleInspectorTextColorCommitted)
		.OnFontSizeCommitted_UObject(this, &UGraphDesignerWidget::HandleInspectorFontSizeCommitted)
		.OnRotationCommitted_UObject(this, &UGraphDesignerWidget::HandleInspectorRotationCommitted)
		.OnPinChanged_UObject(this, &UGraphDesignerWidget::HandleInspectorPinChanged)
		.OnConnectionChanged_UObject(this, &UGraphDesignerWidget::HandleInspectorConnectionChanged)
		.OnAddPin_UObject(this, &UGraphDesignerWidget::HandleInspectorAddPin)
		.OnRemoveLastPin_UObject(this, &UGraphDesignerWidget::HandleInspectorRemoveLastPin)
		.OnDeleteSelected_UObject(this, &UGraphDesignerWidget::HandleInspectorDeleteSelected);

	AnswerPanelWidget = SNew(SGraphDesignerAnswerPanel)
		.Document(Document)
		.OnAnswerChanged_UObject(this, &UGraphDesignerWidget::HandleAnswerChanged)
		.OnAnswerAdded_UObject(this, &UGraphDesignerWidget::HandleAnswerAdded)
		.OnAnswerRemoved_UObject(this, &UGraphDesignerWidget::HandleAnswerRemoved);

	GraphCanvas = SNew(SGraphDesignerCanvas)
		.Document(Document)
		.BackgroundColor(BackgroundColor)
		.GridColor(GridColor)
		.SelectionColor(SelectionColor)
		.ConnectionRoutingMode(ConnectionRoutingMode)
		.OnSelectionChanged_Lambda([this](const TArray<FString>& SelectedElementIds)
		{
			HandleSelectionChanged(SelectedElementIds);
		})
		.OnPinSelected_Lambda([this](const FString& PinId)
		{
			HandlePinSelected(PinId);
		})
		.OnConnectionSelected_Lambda([this](const FString& ConnectionId)
		{
			HandleConnectionSelected(ConnectionId);
		});

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			GraphCanvas.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.WidthOverride(PaletteWidth)
			[
				PaletteWidget.ToSharedRef()
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			SNew(SBox)
			.WidthOverride(InspectorWidth)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(0.62f)
				[
					InspectorWidget.ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.FillHeight(0.38f)
				.Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					AnswerPanelWidget.ToSharedRef()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 8.0f, 8.0f, 4.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("保存图纸")))
					.OnClicked_UObject(this, &UGraphDesignerWidget::OpenSaveDialog)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 4.0f, 8.0f, 8.0f)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("读取图纸")))
					.OnClicked_UObject(this, &UGraphDesignerWidget::OpenLoadDialog)
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Visibility_UObject(this, &UGraphDesignerWidget::GetSaveDialogVisibility)
			.Padding(16.0f)
			[
				SNew(SBox)
				.WidthOverride(360.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("请输入图纸名称")))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 12.0f)
					[
						SAssignNew(SaveNameTextBox, SEditableTextBox)
						.Text(FText::FromString(TEXT("MyNetwork")))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("保存")))
							.OnClicked_UObject(this, &UGraphDesignerWidget::ConfirmSaveDialog)
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("取消")))
							.OnClicked_UObject(this, &UGraphDesignerWidget::CancelSaveDialog)
						]
					]
				]
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Visibility_UObject(this, &UGraphDesignerWidget::GetLoadDialogVisibility)
			.Padding(16.0f)
			[
				SNew(SBox)
				.WidthOverride(360.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("请输入要读取的图纸名称")))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 12.0f)
					[
						SAssignNew(LoadNameTextBox, SEditableTextBox)
						.Text(FText::FromString(TEXT("MyNetwork")))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(0.0f, 0.0f, 6.0f, 0.0f)
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("读取")))
							.OnClicked_UObject(this, &UGraphDesignerWidget::ConfirmLoadDialog)
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("返回")))
							.OnClicked_UObject(this, &UGraphDesignerWidget::CancelLoadDialog)
						]
					]
				]
			]
		];
}

void UGraphDesignerWidget::EnsureDocument()
{
	if (Document == nullptr)
	{
		Document = NewObject<UGraphDocument>(this);
	}
}

void UGraphDesignerWidget::InvalidateCanvas()
{
	if (GraphCanvas.IsValid())
	{
		GraphCanvas->Invalidate(EInvalidateWidget::Paint);
	}
}

FReply UGraphDesignerWidget::OpenSaveDialog()
{
	bSaveDialogVisible = true;
	if (SaveNameTextBox.IsValid())
	{
		SaveNameTextBox->SetText(FText::FromString(TEXT("MyNetwork")));
	}
	return FReply::Handled();
}

FReply UGraphDesignerWidget::ConfirmSaveDialog()
{
	FString GraphName = TEXT("Graph");
	if (SaveNameTextBox.IsValid())
	{
		GraphName = SaveNameTextBox->GetText().ToString().TrimStartAndEnd();
	}

	if (GraphName.IsEmpty())
	{
		GraphName = TEXT("Graph");
	}

	SaveGraph(GraphName);
	bSaveDialogVisible = false;
	return FReply::Handled();
}

FReply UGraphDesignerWidget::CancelSaveDialog()
{
	bSaveDialogVisible = false;
	return FReply::Handled();
}

EVisibility UGraphDesignerWidget::GetSaveDialogVisibility() const
{
	return bSaveDialogVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply UGraphDesignerWidget::OpenLoadDialog()
{
	bLoadDialogVisible = true;
	if (LoadNameTextBox.IsValid())
	{
		LoadNameTextBox->SetText(FText::FromString(TEXT("MyNetwork")));
	}
	return FReply::Handled();
}

FReply UGraphDesignerWidget::ConfirmLoadDialog()
{
	FString GraphName = TEXT("Graph");
	if (LoadNameTextBox.IsValid())
	{
		GraphName = LoadNameTextBox->GetText().ToString().TrimStartAndEnd();
	}

	if (GraphName.IsEmpty())
	{
		GraphName = TEXT("Graph");
	}

	if (LoadGraph(GraphName))
	{
		bLoadDialogVisible = false;
	}
	return FReply::Handled();
}

FReply UGraphDesignerWidget::CancelLoadDialog()
{
	bLoadDialogVisible = false;
	return FReply::Handled();
}

EVisibility UGraphDesignerWidget::GetLoadDialogVisibility() const
{
	return bLoadDialogVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

void UGraphDesignerWidget::HandleSelectionChanged(const TArray<FString>& InSelectedElementIds)
{
	SelectedElementIds = InSelectedElementIds;
	if (InspectorWidget.IsValid())
	{
		InspectorWidget->SetSelectedElementId(SelectedElementIds.Num() > 0 ? SelectedElementIds[0] : TEXT(""));
	}
	if (AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->RebuildPanel();
	}
	OnSelectionChanged.Broadcast(SelectedElementIds);
}

void UGraphDesignerWidget::HandleInspectorTextCommitted(const FString& ElementId, const FString& Text)
{
	SetElementText(ElementId, Text);
}

void UGraphDesignerWidget::HandleInspectorFillColorCommitted(const FString& ElementId, const FLinearColor& FillColor)
{
	SetElementFillColor(ElementId, FillColor);
}

void UGraphDesignerWidget::HandleInspectorBorderColorCommitted(const FString& ElementId, const FLinearColor& BorderColor)
{
	SetElementBorderColor(ElementId, BorderColor);
}

void UGraphDesignerWidget::HandleInspectorTextColorCommitted(const FString& ElementId, const FLinearColor& TextColor)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		SetElementTextStyle(ElementId, TextColor, Element->FontSize);
	}
}

void UGraphDesignerWidget::HandleInspectorFontSizeCommitted(const FString& ElementId, float FontSize)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		SetElementTextStyle(ElementId, Element->TextColor, FontSize);
	}
}

void UGraphDesignerWidget::HandleInspectorRotationCommitted(const FString& ElementId, float Rotation)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		SetElementTransform(ElementId, Element->Position, Element->Size, Rotation);
	}
}

void UGraphDesignerWidget::HandleInspectorPinChanged(const FGraphDesignerPin& Pin)
{
	EnsureDocument();
	Document->UpdatePin(Pin);
	if (AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->RebuildPanel();
	}
	InvalidateCanvas();
}

void UGraphDesignerWidget::HandleInspectorConnectionChanged(const FGraphDesignerConnection& Connection)
{
	EnsureDocument();
	Document->UpdateConnection(Connection);
	InvalidateCanvas();
}

void UGraphDesignerWidget::HandleInspectorAddPin(const FString& ElementId, FVector2D RelativePosition)
{
	EnsureDocument();
	Document->AddPinToElement(ElementId, RelativePosition);
	Document->ArrangePinsOnSameSide(ElementId, RelativePosition);
	InvalidateCanvas();
}

void UGraphDesignerWidget::HandleInspectorRemoveLastPin(const FString& ElementId)
{
	EnsureDocument();
	if (FGraphDesignerElement* Element = Document->FindElement(ElementId))
	{
		if (Element->Pins.Num() > 0)
		{
			const FString PinId = Element->Pins.Last().Id;
			Document->RemovePin(PinId);
			if (AnswerPanelWidget.IsValid())
			{
				AnswerPanelWidget->RebuildPanel();
			}
			InvalidateCanvas();
		}
	}
}

void UGraphDesignerWidget::HandleInspectorDeleteSelected()
{
	DeleteSelectedElements();
}

void UGraphDesignerWidget::HandlePinSelected(const FString& PinId)
{
	if (InspectorWidget.IsValid())
	{
		if (PinId.IsEmpty())
		{
			InspectorWidget->SetSelectedElementId(TEXT(""));
		}
		else
		{
			SelectedElementIds.Reset();
			InspectorWidget->SetSelectedPinId(PinId);
		}
	}
}

void UGraphDesignerWidget::HandleConnectionSelected(const FString& ConnectionId)
{
	if (InspectorWidget.IsValid() && !ConnectionId.IsEmpty())
	{
		SelectedElementIds.Reset();
		InspectorWidget->SetSelectedConnectionId(ConnectionId);
	}
}

void UGraphDesignerWidget::HandleAnswerChanged(int32 Index, const FGraphDesignerRequiredConnection& RequiredConnection)
{
	EnsureDocument();
	Document->UpdateRequiredConnection(Index, RequiredConnection.StartPinId, RequiredConnection.EndPinId);
	if (AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->RebuildPanel();
	}
}

void UGraphDesignerWidget::HandleAnswerAdded()
{
	EnsureDocument();
	Document->AddRequiredConnection(TEXT(""), TEXT(""));
	if (AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->RebuildPanel();
	}
}

void UGraphDesignerWidget::HandleAnswerRemoved(int32 Index)
{
	EnsureDocument();
	Document->RemoveRequiredConnectionAt(Index);
	if (AnswerPanelWidget.IsValid())
	{
		AnswerPanelWidget->RebuildPanel();
	}
}

FString UGraphDesignerWidget::ResolvePinName(const FString& PinId) const
{
	if (Document == nullptr)
	{
		return PinId;
	}

	const FGraphDesignerPin* Pin = Document->FindPin(PinId);
	if (Pin == nullptr || Pin->Id.IsEmpty())
	{
		return PinId;
	}

	return Pin->Id;
}
