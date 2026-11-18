/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#ifndef TK_AREA_EDITOR_H
#define TK_AREA_EDITOR_H
class Assets;
class QMainWindow;
class QDockWidget;
class MapView;
class QToolBar;
class QMenu;
class QAction;
void installAreaEditor(QMainWindow* window, Assets* assets, MapView* map,
                       QToolBar* toolbar, QMenu* menu, QAction* play, QDockWidget* palette);
#endif
