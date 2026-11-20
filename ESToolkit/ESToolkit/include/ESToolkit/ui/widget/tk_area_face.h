/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef TK_AREA_FACE_H
#define TK_AREA_FACE_H
struct AreaPreviewFace {
    int x=0, y=0, layer=0, elevation=0, plane=0;
    bool standing() const { return plane>=1 && plane<=3; }
};
#endif
