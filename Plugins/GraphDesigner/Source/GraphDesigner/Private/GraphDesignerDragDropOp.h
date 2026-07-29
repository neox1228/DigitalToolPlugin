#pragma once

#include "CoreMinimal.h"
#include "Input/DragAndDrop.h"
#include "GraphDesignerTypes.h"


class FGraphDesignerTemplateDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FGraphDesignerTemplateDragDropOp, FDragDropOperation)

	FGraphDesignerNodeTemplate NodeTemplate;

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		return SNew(STextBlock)
			.Text(FText::FromString(NodeTemplate.DisplayName));
	}

	static TSharedRef<FGraphDesignerTemplateDragDropOp> New(const FGraphDesignerNodeTemplate& InNodeTemplate)
	{
		TSharedRef<FGraphDesignerTemplateDragDropOp> Operation = MakeShared<FGraphDesignerTemplateDragDropOp>();
		Operation->NodeTemplate = InNodeTemplate;
		Operation->Construct();
		return Operation;
	}
};
