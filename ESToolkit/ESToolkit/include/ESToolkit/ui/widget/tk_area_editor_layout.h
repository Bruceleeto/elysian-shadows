/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef TK_AREA_EDITOR_LAYOUT_H
#define TK_AREA_EDITOR_LAYOUT_H
#include <QGridLayout>
#include <QSplitter>

// Replace the existing map's grid cell without changing its viewport or scene.
inline QSplitter* createAreaEditorSplitter(QGridLayout* grid, QWidget* map) {
    Q_ASSERT(grid && grid->indexOf(map)>=0);
    int row,column,rowSpan,columnSpan;
    grid->getItemPosition(grid->indexOf(map),&row,&column,&rowSpan,&columnSpan);
    auto* splitter=new QSplitter(Qt::Horizontal,grid->parentWidget());
    splitter->setObjectName("mapEditorSplitter");
    splitter->setChildrenCollapsible(false);
    // A horizontal QSplitter defaults to Preferred vertically. The old map was
    // Expanding, so preserving that policy is essential beside the grid spacer.
    splitter->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    grid->removeWidget(map);
    grid->addWidget(splitter,row,column,rowSpan,columnSpan);
    grid->setRowStretch(row,1);
    return splitter;
}
#endif
