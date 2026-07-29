#include "Core/ChartCanvas.h"
#include "Core/ChartDrawContext.h"
#include "Core/ChartUtils.h"
#include "Widget/ChartWidget.h"
#include "Feature/Modules/ChartModule.h"
#include "Feature/Components/ChartComponent.h"


void SChartCanvas::Construct(const FArguments& InArgs, UChartWidget* InOwner)
{
    Owner = InOwner;
}

int32 SChartCanvas::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
                            const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
                            int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (!Owner.IsValid()) return LayerId;

    // 准备绘制上下文
    ChartDrawContext Ctx(OutDrawElements, AllottedGeometry,LayerId);

    // 1. 绘制背景 (若需要)
    // Ctx.DrawRect(...);

    // 2. 让所有模块绘制（模块可能先绘制网格）
    for (UChartModule* Module : Owner->Modules)
    {
        if (Module)
        {
            Module->CollectDrawElements(Ctx, AllottedGeometry);
        }
    }

    // 3. 让所有组件绘制（系列、轴、提示框等）
    for (UChartComponent* Component : Owner->Components)
    {
        if (Component && Component->bVisible)
        {
            Component->CollectDrawElements(Ctx, AllottedGeometry);
        }
    }

    // 4. 绘制高亮覆盖层（若有）
    // ...

    return Ctx.ReserveLayer(); // 返回最高用到的层数+1
}

FVector2D SChartCanvas::ComputeDesiredSize(float) const
{
    return FVector2D(400.0f, 300.0f);
}

FReply SChartCanvas::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (!Owner.IsValid()) return FReply::Unhandled();
    const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

    // 简易命中测试：遍历所有组件，找到第一个命中的
    int32 NewHitCompIdx = -1;
    int32 NewHitItemIdx = -1;
    for (int32 i = 0; i < Owner->Components.Num(); ++i)
    {
        UChartComponent* Comp = Owner->Components[i];
        if (!Comp || !Comp->bVisible) continue;
        int32 HitIdx = Comp->HitTest(LocalPos, MyGeometry);
        if (HitIdx >= 0)
        {
            NewHitCompIdx = i;
            NewHitItemIdx = HitIdx;
            break;
        }
    }

    // 变化时请求重绘（后续可加入高亮效果）
    if (NewHitCompIdx != LastHitComponentIndex || NewHitItemIdx != LastHitItemIndex)
    {
        LastHitComponentIndex = NewHitCompIdx;
        LastHitItemIndex = NewHitItemIdx;
        Invalidate(EInvalidateWidget::Paint);
    }

    return FReply::Handled();
}

FReply SChartCanvas::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        // 如果有命中项，触发事件
        if (Owner.IsValid() && LastHitComponentIndex >= 0 && LastHitComponentIndex < Owner->Components.Num())
        {
            UChartComponent* HitComp = Owner->Components[LastHitComponentIndex];
            // 这里可以通过委托通知 UChartWidget，进而触发蓝图事件
            // Owner->OnChartItemClicked.Broadcast(HitComp, LastHitItemIndex);
        }
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply SChartCanvas::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // 可以调用所属模块的缩放功能，这里留空，后续实现
    return FReply::Unhandled();
}