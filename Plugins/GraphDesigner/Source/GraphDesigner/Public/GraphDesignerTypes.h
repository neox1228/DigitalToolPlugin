#pragma once

#include "CoreMinimal.h"
#include "GraphDesignerTypes.generated.h"

UENUM(BlueprintType)
enum class EGraphDesignerElementType : uint8
{
	Rectangle UMETA(DisplayName = "Rectangle"),
	RoundedRectangle UMETA(DisplayName = "Rounded Rectangle"),
	Circle UMETA(DisplayName = "Circle"),
	Ellipse UMETA(DisplayName = "Ellipse"),
	Triangle UMETA(DisplayName = "Triangle"),
	RightTriangle UMETA(DisplayName = "Right Triangle"),
	Diamond UMETA(DisplayName = "Diamond"),
	Parallelogram UMETA(DisplayName = "Parallelogram"),
	Trapezoid UMETA(DisplayName = "Trapezoid"),
	Pentagon UMETA(DisplayName = "Pentagon"),
	Hexagon UMETA(DisplayName = "Hexagon"),
	Octagon UMETA(DisplayName = "Octagon"),
	Star UMETA(DisplayName = "Star"),
	Text UMETA(DisplayName = "Text"),
	Image UMETA(DisplayName = "Image"),
	Line UMETA(DisplayName = "Line"),
	Arrow UMETA(DisplayName = "Arrow"),
	Pin UMETA(DisplayName = "Pin")
};

UENUM(BlueprintType)
enum class EGraphDesignerResizeHandle : uint8
{
	None,
	TopLeft,
	Top,
	TopRight,
	Right,
	BottomRight,
	Bottom,
	BottomLeft,
	Left
};

UENUM(BlueprintType)
enum class EGraphDesignerPinTextPosition : uint8
{
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),
	Top UMETA(DisplayName = "Top"),
	Bottom UMETA(DisplayName = "Bottom")
};

UENUM(BlueprintType)
enum class EGraphDesignerConnectionRoutingMode : uint8
{
	ManualRoutePoints UMETA(DisplayName = "Manual Route Points"),
	AutoOrthogonal UMETA(DisplayName = "Auto Orthogonal")
};

UENUM(BlueprintType)
enum class EGraphDesignerConnectionLineStyle : uint8
{
	Solid UMETA(DisplayName = "Solid"),
	Dashed UMETA(DisplayName = "Dashed")
};

USTRUCT(BlueprintType)
struct GRAPHDESIGNER_API FGraphDesignerPin
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString OwnerElementId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FVector2D RelativePosition = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FVector2D Size = FVector2D(12.0f, 12.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor FillColor = FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor BorderColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	EGraphDesignerPinTextPosition TextPosition = EGraphDesignerPinTextPosition::Right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor TextColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner", meta = (ClampMin = "1.0"))
	float FontSize = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	bool bSelected = false;

	FGraphDesignerPin() = default;

	FGraphDesignerPin(const FString& InId, const FString& InOwnerElementId, const FVector2D& InRelativePosition)
		: Id(InId)
		, OwnerElementId(InOwnerElementId)
		, RelativePosition(InRelativePosition)
	{
	}
};

USTRUCT(BlueprintType)
struct GRAPHDESIGNER_API FGraphDesignerElement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	EGraphDesignerElementType Type = EGraphDesignerElementType::Rectangle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FVector2D Position = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FVector2D Size = FVector2D(160.0f, 90.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	float Rotation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor FillColor = FLinearColor(0.18f, 0.22f, 0.28f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor BorderColor = FLinearColor(0.55f, 0.65f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor TextColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner", meta = (ClampMin = "1.0"))
	float FontSize = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString ImagePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FGraphDesignerPin> Pins;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	bool bSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	bool bDragging = false;

	FGraphDesignerElement() = default;

	FGraphDesignerElement(const FString& InId, EGraphDesignerElementType InType, const FVector2D& InPosition, const FVector2D& InSize)
		: Id(InId)
		, Type(InType)
		, Position(InPosition)
		, Size(InSize)
	{
	}
};

USTRUCT(BlueprintType)
struct GRAPHDESIGNER_API FGraphDesignerConnection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString StartPinId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString EndPinId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	EGraphDesignerConnectionLineStyle LineStyle = EGraphDesignerConnectionLineStyle::Solid;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor Color = FLinearColor(0.25f, 0.65f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	float Thickness = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FVector2D> RoutePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	bool bSelected = false;

	FGraphDesignerConnection() = default;

	FGraphDesignerConnection(const FString& InId, const FString& InStartPinId, const FString& InEndPinId)
		: Id(InId)
		, StartPinId(InStartPinId)
		, EndPinId(InEndPinId)
	{
	}
};

USTRUCT(BlueprintType)
struct GRAPHDESIGNER_API FGraphDesignerRequiredConnection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString StartPinId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString EndPinId;

	FGraphDesignerRequiredConnection() = default;

	FGraphDesignerRequiredConnection(const FString& InStartPinId, const FString& InEndPinId)
		: StartPinId(InStartPinId)
		, EndPinId(InEndPinId)
	{
	}
};

USTRUCT(BlueprintType)
struct GRAPHDESIGNER_API FGraphDesignerPinTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FVector2D RelativePosition = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FVector2D Size = FVector2D(12.0f, 12.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor FillColor = FLinearColor(0.05f, 0.05f, 0.05f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor BorderColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	EGraphDesignerPinTextPosition TextPosition = EGraphDesignerPinTextPosition::Right;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor TextColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner", meta = (ClampMin = "1.0"))
	float FontSize = 12.0f;
};

USTRUCT(BlueprintType)
struct GRAPHDESIGNER_API FGraphDesignerNodeTemplate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FName TemplateId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString DisplayName = TEXT("Node");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	EGraphDesignerElementType Type = EGraphDesignerElementType::Rectangle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FVector2D DefaultSize = FVector2D(180.0f, 90.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor FillColor = FLinearColor(0.18f, 0.22f, 0.28f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor BorderColor = FLinearColor(0.55f, 0.65f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner", meta = (ClampMin = "1.0"))
	float FontSize = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString ImagePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FGraphDesignerPinTemplate> Pins;
};

USTRUCT(BlueprintType)
struct GRAPHDESIGNER_API FGraphDesignerDocumentData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	int32 Version = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	FString DocumentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FGraphDesignerElement> Elements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FGraphDesignerConnection> Connections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphDesigner")
	TArray<FGraphDesignerRequiredConnection> RequiredConnections;
};
