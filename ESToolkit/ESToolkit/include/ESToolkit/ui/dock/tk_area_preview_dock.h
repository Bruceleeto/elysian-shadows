/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#ifndef TK_AREA_PREVIEW_DOCK_H
#define TK_AREA_PREVIEW_DOCK_H
class Assets;
class QMainWindow;
class QDockWidget;
QDockWidget* createAreaPreviewDock(QMainWindow* window, Assets* assets);
#endif
