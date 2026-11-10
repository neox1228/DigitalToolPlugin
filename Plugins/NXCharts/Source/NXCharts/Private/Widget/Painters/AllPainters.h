#pragma once

/**
 * 绘制组件汇总头。画布只 include 这一个。
 * 新增组件时：这里加一行 include + ChartCanvas::Construct 里加一行 Add。
 */

#include "Widget/Painters/AnnotationPainter.h"
#include "Widget/Painters/BackgroundPainter.h"
#include "Widget/Painters/GridAndAxesPainter.h"
#include "Widget/Painters/HoverPainter.h"
#include "Widget/Painters/LegendPainter.h"
#include "Widget/Painters/SeriesPainter.h"