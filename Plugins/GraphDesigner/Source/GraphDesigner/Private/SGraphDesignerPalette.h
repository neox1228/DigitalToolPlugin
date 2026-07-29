#pragma once

#include "CoreMinimal.h"
#include "GraphDesignerTypes.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;

class SGraphDesignerPalette : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGraphDesignerPalette)
	{
	}
		SLATE_ARGUMENT(TArray<FGraphDesignerNodeTemplate>, NodeTemplates)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void SetNodeTemplates(const TArray<FGraphDesignerNodeTemplate>& InNodeTemplates);

private:
	void RebuildPalette();

	TArray<FGraphDesignerNodeTemplate> NodeTemplates;
	TSharedPtr<SVerticalBox> ItemBox;
};
