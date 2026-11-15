/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <QESTkUtils/ui/widget/tk_line_chart.h>
#include <QESTkUtils/ui/tk_theme.h>
#include <QEvent>


namespace QESTkUtils {

LineChartView::LineChartView(void):
    QChartView(nullptr, nullptr)
{
    _chart = new QChart();
    _chart->legend()->hide();

    _chart->setTitle("Performance Profiler");
    setChart(_chart);
    setRenderHint(QPainter::Antialiasing);
    _applyTheme();
}

void LineChartView::_applyTheme(void) {
    _chart->setTheme(Theme::isDark()? QChart::ChartThemeDark : QChart::ChartThemeLight);
}

void LineChartView::changeEvent(QEvent* event) {
    if(event->type() == QEvent::PaletteChange) _applyTheme();
    QChartView::changeEvent(event);
}

unsigned LineChartView::addSeries(AbstractFrameUpdatedValueSeries* series) {
    unsigned pos = _series.size();
    _chart->addSeries(series);
    _chart->createDefaultAxes();
    const auto xAxes = _chart->axes(Qt::Horizontal);
    const auto yAxes = _chart->axes(Qt::Vertical);
    if(!xAxes.isEmpty()) {
        xAxes.first()->setRange(0,1000);
        xAxes.first()->setTitleText("Frame");
    }
    if(!yAxes.isEmpty()) {
        yAxes.first()->setRange(0,2000000);
        yAxes.first()->setTitleText("Nanoseconds");
    }
    _series.push_back(series);
    return pos;
}

void LineChartView::onEngineUpdated(void) {
    advanceFrame();
}

void LineChartView::advanceFrame(void) {
    for(auto& it: _series)
        it->update(_curFrame);
    _chart->update();
    repaint();
    if(++_curFrame >= 1000) {
        for(auto& it: _series) it->clear();
        _curFrame = 0;
    }
}




};
